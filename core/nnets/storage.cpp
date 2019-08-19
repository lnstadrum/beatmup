#include "storage.h"
#include "../utils/string_builder.h"

#define STRINGIFY(X) #X

using namespace Beatmup;
using namespace NNets;


/**
	Flags used by Storage::CodeGenerator to keep track of declared data packing functions
*/
enum TypeCoverage{
	PACK_32   = 1 << 1,
	UNPACK_32 = 1 << 2,

	PACK_8FIXED   = 1 << 3,
	UNPACK_8FIXED = 1 << 4,

	PACK_16FIXED   = 1 << 5,
	UNPACK_16FIXED = 1 << 6
};


Storage::Storage(Environment& env, const int width, const int height, const int scalarDepth, const Type type) :
	type(type),
	size(width, height, scalarDepth)
{
	switch (type) {
	case Type::TENSOR:
		tensor = new GL::Tensor(env, width, height, scalarDepth);
		break;

	case Type::TENSOR_8FP_SIGNED:
	case Type::TENSOR_8FP_BRELU6:
		buffer = new GL::StorageBuffer(env, width, height, ceili(scalarDepth, 4) * 4, 1);
		break;

	case Type::TENSOR_16FP_BRELU6:
		buffer = new GL::StorageBuffer(env, width, height, ceili(scalarDepth, 4) * 4, 2);
		break;

	case Type::BUFFER_1D:
		if (size[0] != 1 || size[1] != 1)
			throw DimensionsMismatch(Type::BUFFER_1D);
		buffer = new GL::StorageBuffer(env, size[0], size[1], size[2], sizeof(float));
		break;

	default:
		Insanity::insanity("invalid storage type");
	}
}


Storage::Storage(Environment& env, const int length) :
	type(Type::BUFFER_1D),
	size(1, 1, length)
{
	buffer = new GL::StorageBuffer(env, size[0], size[1], size[2], sizeof(float));
}


Storage::Storage(GL::TextureHandler* texture):
	type(Type::TEXTURE_REFERENCE),
	size(texture->getWidth(), texture->getHeight(), texture->getDepth() * texture->getNumberOfChannels()),
	textureReference(texture)
{}


Storage::Storage(GL::StorageBuffer* buffer):
	type(Type::BUFFER_REFERENCE),
	size(buffer->getWidth(), buffer->getHeight(), buffer->getDepth() * sizeof(float)),
	buffer(buffer)
{
	RuntimeError::check(buffer->getEntrySize() == sizeof(float), "Unsupported buffer type");
}


Storage::~Storage() {
	switch (type) {
	case Type::TENSOR:
		delete tensor;
		break;
	case Type::BUFFER_1D:
	case Type::TENSOR_8FP_SIGNED:
	case Type::TENSOR_8FP_BRELU6:
	case Type::TENSOR_16FP_BRELU6:
		delete buffer;
		break;
	case Type::TEXTURE_REFERENCE:
	case Type::BUFFER_REFERENCE:
		// do nothing; the pointed storage is managed outside
		break;
	default:
		Insanity::insanity("Invalid storage type");
	}
}


void Storage::bind(GraphicPipeline& gpu, GL::ComputeProgram& program, int unit, bool read, bool write) {
	switch (type) {
	case Type::TENSOR:
		gpu.bind(*tensor, unit, read, write);
		break;

	case Type::BUFFER_1D:
	case Type::BUFFER_REFERENCE:
	case Type::TENSOR_8FP_SIGNED:
	case Type::TENSOR_8FP_BRELU6:
	case Type::TENSOR_16FP_BRELU6:
		buffer->allocate(gpu);
		buffer->bind(gpu, unit);
		break;

	case Type::TEXTURE_REFERENCE:
		gpu.bind(*textureReference, unit, TextureParam::INTERP_LINEAR);
		break;
	}
}


std::string Storage::toString(Type type) {
	switch (type) {
	case Beatmup::NNets::Storage::Type::TEXTURE_REFERENCE:
		return "texture reference";
	case Beatmup::NNets::Storage::Type::TENSOR:
		return "tensor";
	case Beatmup::NNets::Storage::Type::BUFFER_1D:
		return "1D buffer";
	case Type::TENSOR_8FP_SIGNED:
		return "8 bit [-0.5..0.5] tensor";
	case Beatmup::NNets::Storage::Type::TENSOR_8FP_BRELU6:
		return "8 bit [0..6] tensor";
	case Beatmup::NNets::Storage::Type::TENSOR_16FP_BRELU6:
		return "16 bit [0..6] tensor";
	default:
		Insanity::insanity("invalid storage type");
	}
	return "unknown";
}


Storage::CodeGenerator::CodeGenerator() : typeCoverage(0) {}


Storage::CodeGenerator& Storage::CodeGenerator::operator()
	(const char* name, Type type, const Size& size, int bindingUnit, const char* qualifiers,
	Access access, const char* accessFuncSuffix)
{
	declare(name, type, size, bindingUnit, qualifiers);
	if (access == Access::LOADING || access == Access::LOADING_AND_STORING)
		addLoadingAccess(name, accessFuncSuffix, type, size);
	if (access == Access::STORING || access == Access::LOADING_AND_STORING)
		addStoringAccess(name, accessFuncSuffix, type, size);
	return *this;
}


Storage::CodeGenerator& Storage::CodeGenerator::declare(const char* name, Type type, const Size& size, int bindingUnit, const char* qualifiers) {
	StringBuilder builder(code);
	switch (type) {
	case Type::TENSOR:
		builder.printf("layout(binding = %d, rgba32f) uniform %s highp image2DArray %s;",
			bindingUnit, qualifiers, name);
		break;

	case Type::TEXTURE_REFERENCE:
		builder.printf("layout(binding = %d, rgba32f) uniform %s highp image2D %s;",
			bindingUnit, qualifiers, name);
		break;

	case Type::BUFFER_1D:
	case Type::BUFFER_REFERENCE:
		builder.printf("layout(binding = %d, std430) %s buffer %s_ { highp vec4 _[]; } %s;",
			bindingUnit, qualifiers, name, name);
		break;

	case Type::TENSOR_8FP_SIGNED:
	case Type::TENSOR_8FP_BRELU6:
		builder.printf("layout(binding = %d, std430) %s buffer %s_ { highp uint _[]; } %s;",
			bindingUnit, qualifiers, name, name);
		break;

	case Type::TENSOR_16FP_BRELU6:
		builder.printf("layout(binding = %d, std430) %s buffer %s_ { highp uvec2 _[]; } %s;",
			bindingUnit, qualifiers, name, name);
		break;
	}

	return *this;
}


Storage::CodeGenerator& Storage::CodeGenerator::addLoadingAccess(const char* name, const char* suffix, Type type, const Size& size) {
	StringBuilder builder(code);

	switch (type) {
	case Type::TENSOR:
	case Type::TEXTURE_REFERENCE:
		builder.printf("highp vec4 loadPacked%s(ivec3 pos) { return imageLoad(%s, pos); }",
			suffix, name).nl();
		builder.printf("highp vec4 unpack%s(vec4 _) { return _; }",
			suffix).nl();
		builder.printf("highp vec4 load%s(ivec3 pos) { return imageLoad(%s, pos); }",
			suffix, name).nl();
		break;

	case Type::BUFFER_1D:
		builder.printf("highp vec4 loadPacked%s(ivec3 pos) { return vec4(%s._[pos.z]); }",
			suffix, name).nl();
		builder.printf("highp vec4 unpack%s(vec4 _) { return _; }",
			suffix).nl();
		builder.printf("highp vec4 load%s(ivec3 pos) { return vec4(%s._[pos.z]); }",
			suffix, name).nl();
		break;

	case Type::TENSOR_8FP_SIGNED:
		// loading
		builder.printf<512>(STRINGIFY(
			highp uint loadPacked%s(ivec3 pos) {
				if (any(lessThan(pos, ivec3(0,0,0))) || any(greaterThanEqual(pos, ivec3(%d,%d,%d))))
					return 0u;
				return uint(%s._[(pos.x * %d + pos.y) * %d + pos.z]);
			}
		),
			suffix,
			size[0], size[1], ceili(size[2], 4),
			name, size[1], ceili(size[2], 4)
		).nl();

		// unpacking
		builder.printf<512>(STRINGIFY(
			highp vec4 unpack%s(highp uint _) {
				highp vec4 r = unpackSnorm4x8(_);
				return 0.5 * r;
			}
		), suffix).nl();

		// loading + unpacking
		builder.printf<512>(STRINGIFY(
			highp vec4 load%s(ivec3 pos) {
				return unpack%s(loadPacked%s(pos));
			}
		), suffix, suffix, suffix).nl();
		break;

	case Type::TENSOR_8FP_BRELU6:
		// loading
		builder.printf<512>(STRINGIFY(
			highp uint loadPacked%s(ivec3 pos) {
				if (any(lessThan(pos, ivec3(0,0,0))) || any(greaterThanEqual(pos, ivec3(%d,%d,%d))))
					return 0u;
				return uint(%s._[(pos.x * %d + pos.y) * %d + pos.z]);
			}
		),
			suffix,
			size[0], size[1], ceili(size[2], 4),
			name, size[1], ceili(size[2], 4)
		).nl();

		// unpacking
		builder.printf<512>(STRINGIFY(
			highp vec4 unpack%s(highp uint _) {
				highp vec4 r = unpackUnorm4x8(_);
				return 6.0f * r;
			}
		), suffix).nl();

		// loading + unpacking
		builder.printf<512>(STRINGIFY(
			highp vec4 load%s(ivec3 pos) {
				return unpack%s(loadPacked%s(pos));
			}
		), suffix, suffix, suffix).nl();
		break;

	case Type::TENSOR_16FP_BRELU6:
		// loading
		builder.printf<512>(STRINGIFY(
			highp uvec2 loadPacked%s(ivec3 pos) {
				if (any(lessThan(pos, ivec3(0,0,0))) || any(greaterThanEqual(pos, ivec3(%d,%d,%d))))
					return uvec2(0, 0);
				return uvec2(%s._[(pos.x * %d + pos.y) * %d + pos.z]);
			}
		),
			suffix,
			size[0], size[1], ceili(size[2], 4),
			name, size[1], ceili(size[2], 4)
		).nl();

		// unpacking
		builder.printf<512>(STRINGIFY(
			highp vec4 unpack%s(highp uvec2 _) {
				highp vec4 r = vec4(unpackUnorm2x16(_.x), unpackUnorm2x16(_.y));
				return 6.0f * r;
			}
		), suffix).nl();

		// loading + unpacking
		builder.printf<512>(STRINGIFY(
			highp vec4 load%s(ivec3 pos) {
				return unpack%s(loadPacked%s(pos));
			}
		), suffix, suffix, suffix).nl();
		break;
	}

	return *this;
}


Storage::CodeGenerator& Storage::CodeGenerator::addStoringAccess(const char* name, const char* suffix, Type type, const Size& size) {
	StringBuilder builder(code);
	switch (type) {
	case Type::TENSOR:
	case Type::TEXTURE_REFERENCE:
		builder.printf("void store%s(ivec3 pos, highp vec4 _) { imageStore(%s, pos, _); }",
			suffix, name).nl();
		break;

	case Type::BUFFER_1D:
		builder.printf("void store%s(ivec3 pos, highp vec4 _) { %s._[pos.z] = _; }",
			suffix, name).nl();
		break;

	case Type::TENSOR_8FP_SIGNED:
		builder.printf(STRINGIFY(
			void store%s(ivec3 pos, highp vec4 _) {
				if (all(greaterThanEqual(pos, ivec3(0, 0, 0))) && all(lessThan(pos, ivec3(%d, %d, %d))))
					%s._[(pos.x * %d + pos.y) * %d + pos.z] = packSnorm4x8(2.0 * _);
			}
		),
			suffix,
			size[0], size[1], ceili(size[2], 4),
			name, size[1], ceili(size[2], 4)
		).nl();
		break;

	case Type::TENSOR_8FP_BRELU6:
		builder.printf(STRINGIFY(
			void store%s(ivec3 pos, highp vec4 _) {
				if (all(greaterThanEqual(pos, ivec3(0, 0, 0))) && all(lessThan(pos, ivec3(%d, %d, %d))))
					%s._[(pos.x * %d + pos.y) * %d + pos.z] = packUnorm4x8(_ / 6.0f);
			}
		),
			suffix,
			size[0], size[1], ceili(size[2], 4),
			name, size[1], ceili(size[2], 4)
		).nl();
		break;

	case Type::TENSOR_16FP_BRELU6:
		builder.printf(STRINGIFY(
			void store%s(ivec3 pos, highp vec4 _) {
				if (all(greaterThanEqual(pos, ivec3(0, 0, 0))) && all(lessThan(pos, ivec3(%d, %d, %d))))
					%s._[(pos.x * %d + pos.y) * %d + pos.z] = uvec2(packUnorm2x16(_.xy / 6.0f), packUnorm2x16(_.zw / 6.0f));
			}
		),
			suffix,
			size[0], size[1], ceili(size[2], 4),
			name, size[1], ceili(size[2], 4))
		.nl();
		break;
	}

	return *this;
}


std::string Storage::getTypeName(Type type) {
	switch (type) {
	case Beatmup::NNets::Storage::Type::TEXTURE_REFERENCE:
	case Beatmup::NNets::Storage::Type::TENSOR:
		return "vec4";
	case Beatmup::NNets::Storage::Type::BUFFER_1D:
		return "vec4";
	case Beatmup::NNets::Storage::Type::TENSOR_8FP_BRELU6:
	case Beatmup::NNets::Storage::Type::TENSOR_8FP_SIGNED:
		return "uint";
	case Beatmup::NNets::Storage::Type::TENSOR_16FP_BRELU6:
		return "uvec2";
	}
	return "";
}


int Storage::getEntrySize(Type type) {
	switch (type) {
	case Beatmup::NNets::Storage::Type::TEXTURE_REFERENCE:
	case Beatmup::NNets::Storage::Type::TENSOR:
		return 4 * 4;
	case Beatmup::NNets::Storage::Type::BUFFER_1D:
		return 4 * 4;
	case Beatmup::NNets::Storage::Type::TENSOR_8FP_BRELU6:
	case Beatmup::NNets::Storage::Type::TENSOR_8FP_SIGNED:
		return 4 * 1;
	case Beatmup::NNets::Storage::Type::TENSOR_16FP_BRELU6:
		return 4 * 2;
	default:
		break;
	}
	return 0;
}