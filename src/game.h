#ifndef DL_CHESS_GAME_H
#define DL_CHESS_GAME_H
#include "SDL3/SDL.h"
#include "structs/AppData.h"
#include "structs/Board.h"
#include "structs/Camera3d.h"
#include "structs/Ray.h"
#include "structs/Vector3.h"

inline void UpdateCamera(AppData* appData)
{
    static float movFactor {2.0f};
    float xMov {0.0f};
    float zMov{0.0f};
    float xCamMov{0.0f};
    float yCamMov{0.0f};
    float camChange = movFactor * appData->deltaTime;
    if(appData->input.pressFlags & PlayerInput::W_PRESSED)
    {
        zMov -= camChange;
    }   
    if(appData->input.pressFlags & PlayerInput::S_PRESSED)
    {
        zMov += camChange;
    }   
    if(appData->input.pressFlags & PlayerInput::A_PRESSED)
    {
        xMov -= camChange;
    }   
    if(appData->input.pressFlags & PlayerInput::D_PRESSED)
    {
        xMov += camChange;
    }   
    if(appData->input.pressFlags & PlayerInput::UP_PRESSED)
    {
        yCamMov += camChange;
    }   
    if(appData->input.pressFlags & PlayerInput::DOWN_PRESSED)
    {
        yCamMov -= camChange;
    }   
    if(appData->input.pressFlags & PlayerInput::LEFT_PRESSED)
    {
        xCamMov -= camChange;
    }   
    if(appData->input.pressFlags & PlayerInput::RIGHT_PRESSED)
    {
        xCamMov += camChange;
    }   
    if(appData->input.pressFlags)
    {
        appData->camera.UpdateCam(xMov, zMov, xCamMov, yCamMov);
        appData->camera.UpdateViewMat();
        appData->camera.UpdateVpMat();
    }
}

inline Vector3 GetSquareClickedOn(AppData* appData)
{
    Ray ray {GetRayFromScreenCoordinate(appData->width, 
                                        appData->height,
                                        appData->input.mouseX,
                                        appData->input.mouseY,
                                        &appData->camera)};
    return GetRayIntersectFlatPlane(ray, Vector3{0.0f,1.0f,0.0f});
}

inline void TryMovePiece(AppData* appData)
{
    Vector3 sqr{GetSquareClickedOn(appData)};
    Board& board{appData->board};
    if (SquareIsOnBoard(sqr.x, sqr.z))
    {
        Piece& t {board.GetPiece(sqr.x, sqr.z)};
        Piece& sP{board.GetSelectedPiece()};

        if((sP.index == EMPTYSQUARE || (!(t.IsNull() || t.flags & Piece::TAKEN))) && t.colour == board.turn)
            board.selectedIndex = t.index;
        else if (sP.CanMoveToSpace(sqr.x, sqr.z))
        {
            board.MoveSelectedPiece(sqr.x, sqr.z);
            sP.TryPromote(appData->assets); 
            board.EndTurn();
        }
        // Try to take piece
        else 
        {
            // check movement is legal
            bool takeIsLegal{false};
            int diffX {(int)sqr.x - sP.bPos.row};
            int diffY {(int)sqr.z - sP.bPos.col};
            if(sP.type == Piece::PAWN)
            {
                takeIsLegal = std::abs(diffX) == 2
                            && ((sP.colour == Piece::BLACK && diffY == 2) || (sP.colour == Piece::WHITE && diffY == -2));
            }
            else if (sP.type == Piece::KING)
            {
                takeIsLegal = std::abs(diffX) == 2 && std::abs(diffY) == 2;
            }
            if(takeIsLegal)
            {
                // Check that there is a piece in the intervening space and that it is of the opponent's colours.
                Piece& inbetweenPiece{board.GetPiece(sP.bPos.row + diffX/2, sP.bPos.col + diffY/2)};
                if(!(inbetweenPiece.IsNull() || inbetweenPiece.flags & Piece::TAKEN) && inbetweenPiece.colour != sP.colour)
                {
                    board.MoveSelectedPiece(sqr.x, sqr.z);
                    board.TakePiece(inbetweenPiece);
                    sP.TryPromote(appData->assets);
                    if(!PieceCanTake(sP, board))
                        board.EndTurn();
                }
            }
        }
    }
}

inline void UpdateCurrentBoard(AppData* appData)
{
    if (appData->input.clickFlags & PlayerInput::LCLICK)
    {
        TryMovePiece(appData);
        appData->input.clickFlags = PlayerInput::LCLICK;
    }
    if (appData->input.clickFlags & PlayerInput::RCLICK)
    {
        appData->board.selectedIndex = EMPTYSQUARE;  
    }
    appData->input.clickFlags = 0;
}

inline void UpdateGame(AppData* appData)
{
    UpdateCamera(appData);
    UpdateCurrentBoard(appData);

}

#endif