#ifndef AUTOFUZZ_CFG_H
#define AUTOFUZZ_CFG_H

#include <cstdint>
#include <vector>
#include <map>

#include <llvm/IR/Function.h>
#include <llvm/IR/CFG.h>

struct IVertex {
    virtual ~IVertex() = default;

    uint32_t id_ = 0;
};

template <typename T>
struct Vertex : IVertex {
    ~Vertex() override = default;

    const T* object_ = nullptr;
};

using VertexPtr  = std::shared_ptr<IVertex>;
using LinkedList = std::vector<VertexPtr>;
struct VertexComparator {
    bool operator()(const VertexPtr& U, const VertexPtr& V) const {
        return U->id_ < V->id_;
    }
};
using AdjacencyList = std::map<VertexPtr, LinkedList, VertexComparator>;

template <typename T>
class CFG {
public:
    explicit CFG()             = default;
    virtual ~CFG()             = default;
    CFG(const CFG&)            = delete;
    CFG& operator=(const CFG&) = delete;
    CFG(CFG&&)                 = delete;
    CFG& operator=(CFG&&)      = delete;

    // Add new node (vertex) to the adjacency list
    // This method is called to initialize (find) all vertices in the function
    void AddVertex(uint32_t id, const T* object) {
        VertexPtr vertex     = std::make_shared<Vertex<T>>();
        auto vertex_data     = static_cast<Vertex<T>*>(vertex.get());

        vertex_data->id_     = id;
        vertex_data->object_ = object;

        adjacency_list_[std::move(vertex)];
    }

    // Add new edge E(u, v) to the adjacency list
    // This method is called when a new neighbor vertex of a vertex was found
    void AddEdge(const T* from, const T* to) {
        VertexPtr U = GetVertexByObject(from);
        if (U == nullptr) {
            return;
        }

        VertexPtr V = GetVertexByObject(to);
        if (V == nullptr) {
            return;
        }

        adjacency_list_[U].push_back(V);
    }

    VertexPtr GetVertexByObject(const T* object) const {
        for (const auto& pair: adjacency_list_) {
            const auto vertex = static_cast<Vertex<T>*>(pair.first.get());
            if (vertex->object_ == object) {
                return pair.first;
            }
        }

        return nullptr;
    }

    const AdjacencyList& GetAdjacencyList() const {
        return adjacency_list_;
    }

private:
    AdjacencyList adjacency_list_;
};

class BasicBlockCFG : public CFG<llvm::BasicBlock> {
public:
    explicit BasicBlockCFG(const llvm::Function& function)
                               : CFG<llvm::BasicBlock>(), function_(function) {}

    ~BasicBlockCFG() override                      = default;
    BasicBlockCFG(const BasicBlockCFG&)            = delete;
    BasicBlockCFG& operator=(const BasicBlockCFG&) = delete;
    BasicBlockCFG(BasicBlockCFG&&)                 = delete;
    BasicBlockCFG& operator=(BasicBlockCFG&&)      = delete;

    const llvm::Function& GetFunction() const {
        return function_;
    }

private:
    const llvm::Function& function_;
};

class FunctionCFG : public CFG<llvm::Function> {
public:
    explicit FunctionCFG() : CFG<llvm::Function>() {}
    ~FunctionCFG() override                    = default;
    FunctionCFG(const FunctionCFG&)            = delete;
    FunctionCFG& operator=(const FunctionCFG&) = delete;
    FunctionCFG(FunctionCFG&&)                 = delete;
    FunctionCFG& operator=(FunctionCFG&&)      = delete;
};

#endif //AUTOFUZZ_CFG_H