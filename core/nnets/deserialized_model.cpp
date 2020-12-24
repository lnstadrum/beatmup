/*
    Beatmup image and signal processing library
    Copyright (C) 2020, lnstadrum

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "deserialized_model.h"
#include <sstream>

using namespace Beatmup;
using namespace NNets;


void DeserializedModel::deserialize(const Listing& listing) {
    InvalidArgument::check(listing.has("ops"), "The listing has no 'ops'");
    InvalidArgument::check(listing.has("connections"), "The listing has no 'connections'");

    const auto& theMap = AbstractOperation::Deserializer::getDeserializersMap();

    // process ops
    const auto& ops = listing["ops"];
    for (const auto& op : ops) {
        // find a deserializer
        const std::string type = op["_type"];
        auto john = theMap.find(type);
        if (john == theMap.end())
            throw InvalidArgument("Cannot find deserializer of type " + type + " declared in operation starting at line " + std::to_string(op.getLineNumber()));

        // create op
        auto newOp = john->second->deserialize(context, op);
        ownedOps.push_back(newOp);
        append(newOp, false);
    }

    // process connections
    const auto conns = listing["connections"];
    for (const auto& conn : conns)
        addConnection(
            conn["from"],
            conn["to"],
            conn.get<int>("from_output", 0),
            conn.get<int>("to_input", 0),
            conn.get<int>("shuffle", 0)
        );
}


DeserializedModel::DeserializedModel(Context& context, const Listing& listing):
    Model(context)
{
    deserialize(listing);
}


DeserializedModel::DeserializedModel(Context& context, const std::string& str):
    Model(context)
{
    std::istringstream strstr(str);
    Listing listing(strstr);
    deserialize(listing);
}


DeserializedModel::~DeserializedModel() {
    for (auto op : ownedOps) {
        ops.erase(std::remove(ops.begin(), ops.end(), op), ops.end());
        delete op;
    }
}