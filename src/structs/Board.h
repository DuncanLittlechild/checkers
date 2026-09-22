#ifndef DL_CHESS_BOARD_H
#define DL_CHESS_BOARD_H
#include "SDL3/SDL.h"
#include "helpers/dl_primitives.h"
#include "structs/AssetManager.h"
#include "structs/Camera3d.h"
#include <array>
#include <cstdlib>
static constexpr int EMPTYSQUARE{0};

struct BPos{
    int row{0};
    int col{0};
};

constexpr std::array<BPos, 25> checkersStartingPositions {{
    // Null piece
    {-1, -1},
    // Player 1 (rows 0-2)
    {0, 0}, {0, 2}, {0, 4}, {0, 6},
    {1, 1}, {1, 3}, {1, 5}, {1, 7},
    {2, 0}, {2, 2}, {2, 4}, {2, 6},

    // Player 2 (rows 5-7)
    {5, 1}, {5, 3}, {5, 5}, {5, 7},
    {6, 0}, {6, 2}, {6, 4}, {6, 6},
    {7, 1}, {7, 3}, {7, 5}, {7, 7},
}};

struct Thing{
    enum ThingType{
        TILE,
        PIECE,
        THINGTYPE_COUNT
    };
    // Rendering data
    GPUMesh* mesh{nullptr};
    Material* material{nullptr};
    Vector3 offset{0.0f, 0.0f, 0.0f};
    float scale{};

    ThingType thingType{};
    BPos bPos{};
    Uint8 flags{};

protected:
    Thing(ThingType g_thingType)
        : thingType{g_thingType}
    {}
};

struct Tile : public Thing {
    // Array index of the piece occupying the tile.
    int pieceIndex{};

    Tile()
        : Thing{ThingType::TILE}
    {}

    bool IsEmpty()
    {
        return pieceIndex == 0;
    }
};

struct Piece : public Thing {
        // NULL_COL should only be used on the null piece
    enum Colour{
        NULL_COL,
        WHITE,
        BLACK,
        COLOUR_COUNT
    };

    enum Flags : Uint8 {
        TAKEN = 0b0000'0001,
    };

    enum Type {
        PAWN,
        KING,
    };

    int index{};
    Type type{};
    Colour colour{};

    Piece()
        : Thing {ThingType::PIECE}
    {} 

    bool CanMoveToSpace(int newRow, int newCol)
    {
        switch (type){
            case(PAWN):{
                return std::abs(newRow - bPos.row) == 1 && ((colour == BLACK && (newCol - bPos.col) == 1) || (colour == WHITE && (newCol - bPos.col) == -1));
            }break;
            case(KING):{
                return std::abs(newRow - bPos.row) == 1 && std::abs(newCol - bPos.col) == 1;
            } break;
        }
    }

    bool IsNull()
    {
        return index == 0;
    }

    void Take()
    {
        flags |= TAKEN;
    }

    bool IsTaken()
    {
        return flags & TAKEN;
    }

    bool TryPromote(AssetManager& assets)
    {
        if((colour == BLACK && bPos.col == 7) || (colour == WHITE && bPos.col == 0))
        {
            type = KING;
            mesh = &assets.king;
            return true;
        }
        return false;
    }
};

struct Board{
    static constexpr int PIECESPERPLAYER {12};
    std::array<std::array<Tile, 8>, 8> board{};
    std::array<Piece, 25> pieces{};

    int selectedIndex{0};
    Piece::Colour turn {Piece::WHITE};

    void Init(AssetManager& assets)
    {

        // Generate tiles
        Tile tile{};
        tile.mesh = &assets.tile;
        tile.scale = 1.0f;
        bool whiteTile{false};
        for (int col{0}; col < board.size(); ++col)
        {
            for (int row{0}; row < board[col].size(); ++row)
            {
                tile.material = whiteTile ? &assets.tileMatW : &assets.tileMatB;
                tile.bPos.col = col;
                tile.bPos.row = row;

                tile.offset.x = row + 0.5f;
                tile.offset.z = col + 0.5f;

                board[col][row] = tile;
                whiteTile = !whiteTile;
            }
            whiteTile = !whiteTile;
        }

        // generate pieces
        Piece piece{};
        piece.offset.y = CHECKERSHEIGHT/2;
        piece.mesh = &assets.tile;
        piece.scale = 0.8f;
        // Push null Piece
        pieces[0] = piece;
        for (int i {1}; i < checkersStartingPositions.size(); ++i)
        {
            auto& pos {checkersStartingPositions[i]};
            piece.index = i;
            piece.bPos = pos;
            piece.offset.x = pos.row;
            piece.offset.z = pos.col;
            if (i < 13)
            {
                piece.colour = Piece::BLACK;
                piece.material = &assets.pieceMatB;
            }
            else
            {
                piece.colour = Piece::WHITE;
                piece.material = &assets.pieceMatW;
            }
            pieces[i] = piece;
            board[pos.col][pos.row].pieceIndex = i;
        }
    }

    void MoveSelectedPiece(int newRow, int newCol)
    {
        Piece& selectedPiece {pieces[selectedIndex]};
        board[selectedPiece.bPos.col][selectedPiece.bPos.row].pieceIndex = 0;
        selectedPiece.bPos.row = newRow;
        selectedPiece.bPos.col = newCol;
        board[selectedPiece.bPos.col][selectedPiece.bPos.row].pieceIndex = selectedPiece.index;
    }

    Tile& operator[](int y, int x)
    {
        return board[y][x];
    }  

    Piece& GetPiece(int x, int y)
    {
        return pieces[board[y][x].pieceIndex];
    }

    Piece& GetSelectedPiece()
    {
        return pieces[selectedIndex];
    }

    void EndTurn()
    {
        turn = turn == Piece::WHITE ? Piece::BLACK : Piece::WHITE;
        selectedIndex = 0;
    }

    void TakePiece(Piece& toTake)
    {
        toTake.flags |= Piece::TAKEN;
        board[toTake.bPos.col][toTake.bPos.row].pieceIndex = 0;
    }
};

inline bool SquareIsOnBoard(int x, int y)
{
    return x < 8 && x >= 0 && y < 8 && y >= 0;

}

inline bool PieceCanTake(Piece& p, Board& board)
{
    static constexpr std::pair<int, int> PAWNSQUARES[2]{{1, -1}, {1, 1}};
    static constexpr std::pair<int, int> KINGSQUARES[4]{{1, -1}, {1, 1}, {-1, -1}, {-1, 1}};

    //Check that places 2 squares away can be moved to
    std::vector<std::pair<int, int>> freeSquares{};
    if(p.type == Piece::PAWN)
    {
        freeSquares.reserve(2);

        int mDir {p.colour == Piece::WHITE ? -2 : 2};
        int newRowArr[2] {p.bPos.row + 2, p.bPos.row - 2};
        int newCol {p.bPos.col + mDir};
        for (auto& x : newRowArr)
        {
            if (SquareIsOnBoard(x, newCol))
            {
                Piece& target{board.GetPiece(x, newCol)};
                if(target.IsNull() || target.IsTaken())
                    freeSquares.emplace_back(std::pair<int, int>{newCol, x});
            }
        }
    }
    else if (p.type == Piece::KING)
    {
        freeSquares.reserve(4);
        int newRowArr[2] {p.bPos.row + 2, p.bPos.row - 2};
        int newColArr[2] {p.bPos.col + 2, p.bPos.col - 2};
        for (auto& y : newColArr)
        {
            for (auto& x: newRowArr)
            {
                if(SquareIsOnBoard(x, y))
                {
                    Piece& target{board.GetPiece(x, y)};
                    if (target.IsNull() || target.IsTaken())
                        freeSquares.emplace_back(std::pair<int,int>(y, x));
                }
            }
        }
    }
    // Check that those spaces have another piece inbetween them and the selected piece.
    for (auto& sqr : freeSquares)
    {
        int diffCol {sqr.first - p.bPos.col > 0 ? 1 : -1};
        int diffRow {sqr.second - p.bPos.row > 0 ? 1 : -1};
        int iCol {p.bPos.col + diffCol};
        int iRow {p.bPos.row + diffRow};
        Piece& inbetweenPiece{board.GetPiece(iRow, iCol)};
        if(!(inbetweenPiece.IsNull() || inbetweenPiece.IsTaken()) && inbetweenPiece.colour != p.colour)
            return true;
    }
    return false;
}

#endif