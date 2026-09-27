#ifndef DL_CHESS_BOARD_H
#define DL_CHESS_BOARD_H
#include "SDL3/SDL.h"
#include "helpers/dl_primitives.h"
#include "structs/AssetManager.h"
#include "structs/RenderData.h"
#include <array>
#include <cstdlib>
static constexpr int EMPTYSQUARE{0};

struct BPos{
    int row{0};
    int col{0};

    BPos operator+(BPos r)
    {
        r += *this;
        return r;
    }

    BPos operator-()
    {
        BPos tmp{*this};
        tmp.col = -tmp.col;
        tmp.row = -tmp.row;
        return tmp;
    }

    BPos operator-(BPos r)
    {
        return *this + -r;
    }

    BPos& operator+=(const BPos& r)
    {
        row += r.row;
        col += r.col;
        return *this;
    }

    BPos& operator*=(int r)
    {
        row *= r;
        col *= r;
        return *this;
    }

    BPos& operator/=(int r)
    {
        row /= r;
        col /= r;
        return *this;
    }
};

inline BPos operator*(BPos l, int r)
{
    l *= r;
    return l;
}

inline BPos operator/(BPos l, int r)
{
    l /= r;
    return l;
}

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
    enum ThingType : Uint8{
        TILE,
        PIECE,
        THINGTYPE_COUNT
    };
    // Rendering data
    RenderData renderData{};

    ThingType thingType{TILE};
    BPos bPos{0,0};
    Uint8 flags{0};

protected:
    Thing(ThingType g_thingType)
        : thingType{g_thingType}
    {}
};

struct Tile : public Thing {
    // Array index of the piece occupying the tile.
    enum Flags : Uint8 {
        POSSIBLEMOVE = 0b0000'0001,
        POSSIBLETAKE = 0b0000'0010
    };
    int pieceIndex{0};
    Uint8 flags{0};

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
        SELECTED = 0b0000'0010
    };

    enum Type {
        PAWN,
        KING,
    };

    int index{0};
    Type type{PAWN};
    Colour colour{NULL_COL};

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
            default:{
                assert(false && "Unexpected additional piece type checked for movement");
            };
        }
    }

    bool IsNull()
    {
        return index == 0;
    }

    void MoveTo(BPos& newPos)
    {
        BPos diff {newPos - bPos};
        bPos = newPos;
        renderData.offset.x += diff.row;
        renderData.offset.z += diff.col;
    }

    void Take()
    {
        flags |= TAKEN;
    }

    void Select(AssetManager* assets)
    {
        flags |= SELECTED;
        renderData.material = &assets->pieceMatSelected;
    }

    bool IsSelected()
    {
        return flags & SELECTED;
    }

    void DeSelect(AssetManager* assets)
    {
        flags &= ~SELECTED;
        renderData.material = colour == WHITE ? &assets->pieceMatW : &assets->pieceMatB;
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
            renderData.mesh = &assets.king;
            return true;
        }
        return false;
    }
};

template <typename T, std::size_t W, std::size_t H> 
struct Array1D{
    std::array<T, H*W> data{};

    T& operator[](int col, int row)
    {
        return data[col * W + row];
    }

    constexpr std::size_t ColSize() noexcept
    {
        return H;
    }

    constexpr std::size_t RowSize() noexcept
    {
        return W;
    }
};


inline bool SquareIsOnBoard(int x, int y)
{
    return x < 8 && x >= 0 && y < 8 && y >= 0;

}

struct Board{
    static constexpr int PIECESPERPLAYER {12};
    Array1D<Tile, 8, 8> board{};
    std::array<Piece, 25> pieces{};

    int selectedIndex{0};
    float squareClickedOnX{.0f};
    float squareClickedOnZ{.0f};
    Piece::Colour turn {Piece::WHITE};

    void Init(AssetManager& assets)
    {
        // Generate tiles
        Tile tile{};
        tile.renderData.mesh = &assets.tile;
        tile.renderData.scale = 1.0f;
        bool whiteTile{false};
        for (int col{0}; col < board.ColSize(); ++col)
        {
            for (int row{0}; row < board.RowSize(); ++row)
            {
                tile.renderData.material = whiteTile ? &assets.tileMatW : &assets.tileMatB;
                tile.bPos.col = col;
                tile.bPos.row = row;

                tile.renderData.offset.x = row + 0.5;
                tile.renderData.offset.z = col + 0.5;

                board[col, row] = tile;
                whiteTile = !whiteTile;
            }
            whiteTile = !whiteTile;
        }

        // generate pieces
        Piece piece{};
        piece.renderData.offset.y = CHECKERSHEIGHT/2;
        piece.renderData.mesh = &assets.tile;
        piece.renderData.scale = 0.8f;
        // Push null Piece
        pieces[0] = piece;
        for (int i {1}; i < checkersStartingPositions.size(); ++i)
        {
            auto& pos {checkersStartingPositions[i]};
            piece.index = i;
            piece.bPos = pos;
            piece.renderData.offset.x = pos.row + 0.5;
            piece.renderData.offset.z = pos.col + 0.5;
            if (i < 13)
            {
                piece.colour = Piece::BLACK;
                piece.renderData.material = &assets.pieceMatB;
            }
            else
            {
                piece.colour = Piece::WHITE;
                piece.renderData.material = &assets.pieceMatW;
            }
            pieces[i] = piece;
            board[pos.col,pos.row].pieceIndex = i;
        }
    }

    void IdentifyPossibleDestinations(Piece& newPiece)
    {
        // Can move to all empty tiles within one square (only forwards if a pawn) of all empty tiles within two
        // squares if there is an enemy piece inbetween

        static constexpr BPos BLACKPAWNSQUARES[2]{{1, -1}, {1, 1}};
        static constexpr BPos WHITEPAWNSQUARES[2]{{-1, -1}, {-1, 1}};
        static constexpr BPos KINGSQUARES[4]{{1, -1}, {1, 1}, {-1, -1}, {-1, 1}};

        // Setup the arry of position offsets based on piece type and colour
        std::vector<BPos> squares{};
        squares.reserve(4);
        if (newPiece.type == Piece::KING)
        {
            for (auto& sq : KINGSQUARES)
                squares.push_back(sq);
        }
        else if (newPiece.colour == Piece::BLACK)
        {
            for (auto& sq : BLACKPAWNSQUARES)
                squares.push_back(sq);
        }
        else
        {
            for (auto& sq : WHITEPAWNSQUARES)
                squares.push_back(sq);
        }

        // Check neighbouring spaces. If they are empty, set as possible destination. If not, check the next square on
        // to see if it is empty
        for (auto& mv : squares)
        {
            BPos newPos {newPiece.bPos + mv};
            if(!SquareIsOnBoard(newPos.row, newPos.col))
                continue;
            Tile& dest {board[newPos.col, newPos.row]};
            if (dest.pieceIndex == EMPTYSQUARE)
            {
                dest.flags |= Tile::POSSIBLEMOVE;
            }
            // If the next square on is occupied
            else if (pieces[dest.pieceIndex].colour != turn)
            {
                BPos newPos2 {newPiece.bPos + mv * 2};
                if (Tile& dest {board[newPos2.col, newPos2.row]}; dest.pieceIndex == EMPTYSQUARE)
                {
                    dest.flags |= Tile::POSSIBLETAKE;
                }
            }
        }
    }

    // Tile flags must be reset whenever the tile of the selected piece changes
    // This can either be because: the selected piece has moved; a new piece has been
    // selected; or the turn has changed.
    void ResetTileFlags()
    {
        for (auto& tile : board.data)
        {
            tile.flags &= ~(Tile::POSSIBLEMOVE | Tile::POSSIBLETAKE);
        }
    }

    void DeSelectPiece(AssetManager* assets)
    {
        ResetTileFlags();
        GetSelectedPiece().DeSelect(assets);
        selectedIndex = EMPTYSQUARE;
    }

    void SelectPiece(Piece& newPiece, AssetManager* assets)
    {
        if (selectedIndex != EMPTYSQUARE)
            DeSelectPiece(assets);
        newPiece.Select(assets);
        selectedIndex = newPiece.index;
        IdentifyPossibleDestinations(newPiece);
    }

    void MovePieceToTile(Piece& piece, Tile& tile, AssetManager& assets)
    {
        // Reset move and take flags
        ResetTileFlags();
        // Empty the piece index of the tile the piece is moving from
        board[piece.bPos.col, piece.bPos.row].pieceIndex = EMPTYSQUARE;
        // move the piece 
        piece.MoveTo(tile.bPos);
        tile.pieceIndex = piece.index;
        piece.TryPromote(assets);
        IdentifyPossibleDestinations(piece);
    }

    void TakeToTile(Piece& piece, Tile& tile, AssetManager& assets)
    {
        BPos offset {((tile.bPos - piece.bPos)/2) + piece.bPos};
        Piece& toTake {GetPiece(offset.row, offset.col)};
        TakePiece(toTake);
        MovePieceToTile(piece, tile, assets);
    }

    bool SelectedPieceCanTake()
    {
        for(auto& tile : board.data)
        {
            if (tile.flags & Tile::POSSIBLETAKE)
                return true;
        }
        return false;
    }


    Tile& operator[](int y, int x)
    {
        return board[y,x];
    }  

    Piece& GetPiece(int x, int y)
    {
        return pieces[board[y,x].pieceIndex];
    }

    Piece& GetSelectedPiece()
    {
        return pieces[selectedIndex];
    }

    void EndTurn(AssetManager* assets)
    {
        DeSelectPiece(assets);
        ResetTileFlags();
        turn = turn == Piece::WHITE ? Piece::BLACK : Piece::WHITE;
        selectedIndex = 0;
    }

    void TakePiece(Piece& toTake)
    {
        toTake.flags |= Piece::TAKEN;
        board[toTake.bPos.col,toTake.bPos.row].pieceIndex = 0;
        toTake.bPos = BPos{-1, -1};
    }
};

#endif