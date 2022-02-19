#ifndef AUTOFUZZ_CFG_H
#define AUTOFUZZ_CFG_H

#include <cstdint>
#include <vector>
#include <map>

#include <llvm/IR/Function.h>
#include <llvm/IR/CFG.h>

//
// A node of a graph
//
struct Vertex {
    // A unique identification for a basic block
    uint32_t                id_    = 0;
    // Actual basic block pointer
    const llvm::BasicBlock* block_ = nullptr;
};

//
// An adjacency list structure:
// std::vector
//     |__std::map
//           |__Vertex
//           |__std::vector<Vertex>
//           |__Comparator (additionally needed for user-defined keys)
using List = std::map<Vertex, std::vector<Vertex>,
        decltype([](const Vertex& U, const Vertex& V) -> bool {
            return U.id_ < V.id_;
        })>;
using AdjacencyList = std::vector<List>;

//
// A class-container for an adjacency list of a function
//
class CFG {
public:
    explicit CFG(const llvm::Function&);
    explicit CFG(const llvm::Function&, uint64_t);

    ~CFG()                     = default;
    CFG(const CFG&)            = delete;
    CFG& operator=(const CFG&) = delete;
    CFG(CFG&&)                 = delete;
    CFG& operator=(CFG&&)      = delete;

    // Add new node (vertex) to the adjacency list
    // This method is called to initialize (find) all vertices in the function
    void AddVertex(uint32_t, const llvm::BasicBlock*);

    // Add new edge E(u, v) to the adjacency list
    // This method is called when a new neighbor vertex of a vertex was found
    void AddEdge(const llvm::BasicBlock*, const llvm::BasicBlock*);

    // --------------------------------------------------------------------- //
    //                              Getters                                  //
    // --------------------------------------------------------------------- //
    const llvm::Function& GetFunction() const;
    const AdjacencyList& GetAdjacencyList() const;
    std::string GetFunctionName() const;

private:
    const llvm::Function& function_;
    AdjacencyList         adjacency_list_;

    // Return the vertex structure to the corresponding basic block
    // This method is called when all vertices have been previously identified
    const Vertex& GetVertexByBlock(const llvm::BasicBlock*);
};

#endif //AUTOFUZZ_CFG_H