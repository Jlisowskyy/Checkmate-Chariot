//
// Created by Jlisowskyy on 2/26/24.
//

#ifndef MOVETRIE_H
#define MOVETRIE_H

#include <string>
#include <vector>

/*
 *  Possibly in the future, point structure will be reconsidered.
 *  Especially in a situation when the current one will not be
 *  efficient enough.
 */

class MoveTRIE
{
    // ------------------------------
    // Class inner Types
    // ------------------------------

    struct node
    {
        node()  = default;
        ~node() = default;

        std::vector<std::string> _moves{};
        std::vector<node *> _next;
    };

    // ------------------------------
    // Class creation
    // ------------------------------

    public:
    MoveTRIE() { _root = new node(); }

    ~MoveTRIE() { _destroy(_root); }

    // to allow some more complicated initialization
    MoveTRIE(MoveTRIE &&)            = default;
    MoveTRIE &operator=(MoveTRIE &&) = default;

    MoveTRIE &operator=(const MoveTRIE &) = delete;
    MoveTRIE(const MoveTRIE &)            = delete;

    // ------------------------------
    // Class interaction
    // ------------------------------

    void AddMoveSequence(const std::vector<std::string> &moves)
        // should only be used on building phase, because OptimiseMemoryUsage should be run after all
        ;

    void OptimiseMemoryUsage()
        // should be used after adding all move sequences to the database to optimize memory usage
        ;

    [[nodiscard]] const std::vector<std::string> &FindNextMoves(const std::vector<std::string> &moves) const
        // used to query the database about next save moves that could be added to sequence.
        // return empty vector if no moves are possible
        ;

    // ------------------------------
    // Private class methods
    // ------------------------------

    private:
    [[nodiscard]] static ssize_t _findOnArr(const std::string &move, const std::vector<std::string> &moves)
        // checks whether passed move is present on moves' vector
        // returns index of the found element or -1 if not found.
        ;

    static void _destroy(const node *root)
        // simply deletes all underlying nodes under root with root considered
        ;

    static void _optMem(node *root)
        // simply runs .shrink_to_fit() on every underlying node component
        ;

    // ------------------------------
    // Class fields
    // ------------------------------

    node *_root{};
};

#endif // MOVETRIE_H
