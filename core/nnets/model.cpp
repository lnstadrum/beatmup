#include "model.h"

using namespace Beatmup;
using namespace NNets;


Model::Connection::Connection(AbstractOperation* source, int output, int input, AbstractOperation* dest) :
	source(source), dest(dest), output(output), input(input)
{
	if (source->getOutputSize(output) != dest->getInputSize(input))
		throw OperationSizeMismatch(*this);

	if (!dest->setInputType(source->getOutputType(output)))
		throw OperationInputTypeMismatch(*this);
}


Model::OperationSizeMismatch::OperationSizeMismatch(const Connection& conn) :
	Exception("Size mismatch when linking a model: %s->%d [%d,%d,%d] does not match %d->%s [%d,%d,%d]",
		conn.getSource()->getName().c_str(), conn.getSourceOutput(),
		conn.getSource()->getOutputSize(conn.getSourceOutput())[0],
		conn.getSource()->getOutputSize(conn.getSourceOutput())[1],
		conn.getSource()->getOutputSize(conn.getSourceOutput())[2],
		conn.getDestinationInput(), conn.getDestination()->getName().c_str(),
		conn.getDestination()->getInputSize(conn.getDestinationInput())[0],
		conn.getDestination()->getInputSize(conn.getDestinationInput())[1],
		conn.getDestination()->getInputSize(conn.getDestinationInput())[2]
	)
{}


Model::OperationInputTypeMismatch::OperationInputTypeMismatch(const Connection& conn) :
	Exception("Input type mismatch when linking a model: %s->%d of %s is not accepted by %d->%s",
		conn.getSource()->getName().c_str(), conn.getSourceOutput(),
		Storage::toString( conn.getSource()->getOutputType(conn.getSourceOutput()) ).c_str(),
		conn.getDestinationInput(), conn.getDestination()->getName().c_str())
{}


Model::Model(const std::string& chunkFleName):
	data(chunkFleName)
{}


Model::~Model() {
	connections.clear();
	for (auto& op : ops)
		delete op;
}


void Model::addOperation(AbstractOperation* operation) {
	if (!ops.empty())
		connections.emplace_back(ops.back(), 0, 0, operation);
	ops.push_back(operation);
	changeCounter++;
}



void Model::addOperation(AbstractOperation* operation, AbstractOperation* source, int output, int input) {
	connections.emplace_back(source, output, input, operation);
	ops.push_back(operation);
	changeCounter++;
}


void Model::buildGraph(Graph& graph, GraphKind graphKind) const {
	// map operation pointers to indices to go N*log(M) with N connections and M ops
	std::map<AbstractOperation*, size_t> opIdx;
	for (size_t i = 0; i < ops.size(); ++i)
		opIdx[ops[i]] = i;

	// compute lane sizes
	const size_t outputsShift = graphKind == GraphKind::BOTH ? ops.size() : 0;
	std::vector<int> sizes(ops.size() + outputsShift, 0);
	switch (graphKind) {
	case GraphKind::INPUTS:
	case GraphKind::BOTH:
		for (const auto& conn : connections) {
			auto it = opIdx.find(conn.getDestination());
			BEATMUP_ASSERT_DEBUG(it != opIdx.end());
			sizes[it->second] ++;
		}
		if (graphKind == GraphKind::INPUTS)
			break;
	//fallthrough for GraphKind::BOTH

	case GraphKind::OUTPUTS:
		for (const auto& conn : connections) {
			auto it = opIdx.find(conn.getSource());
			BEATMUP_ASSERT_DEBUG(it != opIdx.end());
			sizes[outputsShift + it->second] ++;
		}
		break;

	default:
		Insanity::insanity("Invalid graph kind");
	}

	// allocate
	graph.resize(sizes.size());
	for (size_t i = 0; i < graph.size(); ++i) {
		graph[i].reserve(sizes[i]);
		graph[i].resize(0);
	}

	// go
	switch (graphKind) {
	case GraphKind::INPUTS:
	case GraphKind::BOTH:
		for (const auto& conn : connections)
			graph[opIdx[conn.getDestination()]].emplace_back(GraphConnection{
				opIdx[conn.getSource()],
				conn.getSourceOutput(),
				conn.getDestinationInput()
			});
		if (graphKind == GraphKind::INPUTS)
			break;
	//fallthrough for GraphKind::BOTH

	case GraphKind::OUTPUTS:
		for (const auto& conn : connections)
			graph[outputsShift + opIdx[conn.getSource()]].emplace_back(GraphConnection{
				opIdx[conn.getDestination()],
				conn.getSourceOutput(),
				conn.getDestinationInput()
			});
		break;
	}
}


AbstractOperation* Model::getOperation(const std::string& name) const {
	for (const auto& op : ops)
		if (op->getName() == name)
			return op;
	return nullptr;
}


void Model::fetchConnection(size_t index, AbstractOperation*& source, AbstractOperation*& destination, int& sourceOutput, int& destinationInput) const {
	const auto& conn = connections[index];
	source = conn.getSource();
	destination = conn.getSource();
	sourceOutput = conn.getSourceOutput();
	destinationInput = conn.getDestinationInput();
}