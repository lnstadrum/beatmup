/*
    Neural net
    A bundle of operations and their connections to each other
*/

#pragma once

#include "ops/operation.h"
#include "../../exception.h"
#include "../../utils/chunkfile.h"
#include <vector>

namespace Beatmup {
    namespace NNets {
        class Model {
        private:
            class Connection {
            private:
                AbstractOperation* source;
                AbstractOperation* dest;
                int output;
                int input;
            public:
                Connection(AbstractOperation* source, int output, int input, AbstractOperation* dest);

                AbstractOperation* getSource() const { return source; }
                AbstractOperation* getDestination() const { return dest; }
                int getSourceOutput() const { return output; }
                int getDestinationInput() const { return input; }
            };

            std::vector<AbstractOperation*> ops;
            std::vector<Connection> connections;

            ChunkFile data;
            
            msize changeCounter;

        public:
            typedef struct {
                size_t operationIndex;
                int sourceOutput;
                int destinationInput;
            } GraphConnection;

            /**
                Model graph
                Depending on the graph kind, specifies linking of inputs/outputs of an operation
                to outputs/inputs of every other operation connected to the current operation.
                A list carrying at i-th entry the list of connections of the i-th operation.
                1st tuple entry: the other operation index.
                2nd tuple entry: the source operation output index.
                3rd tuple entry: the destination operation input index.

                Example (4 ops, 3 connections):
                           + - - - - +                               + - - - - +
                           |   Op1   | <-(n-th in)-------(m-th out)- +   Op0   +
                           + - - - - +                               + - - - - +
                           /         \                                  /
                       i-th out     j-th out                        r-th out
                         /             \                              /
                        /               \                            /
                       /                 \                          /
                   k-th in              l-th in                    /
                     /                     \                      /
                    v                       v                    /
                + - - - - +            + - - - - +              /
                |   Op2   |            |   Op3   | <-(s-th in)-'
                + - - - - +            + - - - - +

                Inputs graph:
                    0 (Op0): []
                    1 (Op1): [(0, m, n)]
                    2 (Op2): [(1, i, k)]
                    3 (Op3): [(0, r, s), (1, j, l)]
                
                Outputs graph:
                    0 (Op0): [(1, m, n), (3, r, s)]
                    1 (Op1): [(2, i, k), (3, j, l)]
                    2 (Op2): []
                    3 (Op3): []

            */
            typedef std::vector<std::vector<GraphConnection>> Graph;
            

            /**
                Tells the connections direction used in the graph
            */
            enum class GraphKind {
                INPUTS,		//!< the graph stores input connections for every operation
                OUTPUTS,	//!< the graph stores output connections for every operation
                BOTH		//!< the graph consists of inputs and outputs graphs concatenated
            };


            class OperationSizeMismatch : public Exception {
            public:
                OperationSizeMismatch(const Connection& conn);
            };

            class OperationInputTypeMismatch : public Exception {
            public:
                OperationInputTypeMismatch(const Connection& conn);
            };

            Model(const std::string& chunkFleName);
            ~Model();

            void addOperation(AbstractOperation* operation);
            void addOperation(AbstractOperation* operation, AbstractOperation* source, int output = 0, int input = 0);

            /**
                Builds model graph
                \param[out] graph		The graph to fill
                \param[in] graphKind	Specifies graph directions (inputs or outputs)
            */
            void buildGraph(Graph& graph, GraphKind graphKind) const;

            size_t getOperationsCount() const  { return ops.size(); }
            size_t getConnectionsCount() const { return connections.size(); }

            AbstractOperation* getOperation(size_t index) const { return ops[index]; }
            AbstractOperation* getOperation(const std::string& name) const;
            void fetchConnection(size_t index, AbstractOperation*& source, AbstractOperation*& destination, int& sourceOutput, int& destinationInput) const;
                        
            inline ChunkFile& getData() { return data; }

            /**
                Provides a model revision number allowing to identify whether there have been any
                changes in the model.
                \return revision number.
            */
            msize getRevision() const { return changeCounter; }

            const AbstractOperation& getLastAddedOp() const { return *ops.back(); }
        };
    }
}
