#ifndef DL_CHESS_GAME_H
#define DL_CHESS_GAME_H
#include "SDL3/SDL.h"
#include "structs/AppData.h"
#include "structs/Board.h"
#include "structs/Camera3d.h"
#include "structs/Ray.h"
#include "structs/Vector3.h"
#include <X11/Xmd.h>

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

inline bool ResolveLeftClick(AppData* appData)
{
    Vector3 sqr{GetSquareClickedOn(appData)};
    appData->board.squareClickedOnX = sqr.x;
    appData->board.squareClickedOnZ = sqr.z;
    Board& board{appData->board};
    // if the square selected is not on the board, nothing can be done
    if(!SquareIsOnBoard(sqr.x, sqr.z))
        return false;

    bool endTurn{false};
    Piece& targetPiece {board.GetPiece(sqr.x, sqr.z)};
    // If another piece is selected, the only possible action to take is to try and select it
    if (targetPiece.index != EMPTYSQUARE && targetPiece.colour == board.turn)
    {
        board.SelectPiece(targetPiece, &appData->assets);
    }
    // If an empty square is selected and a piece is currently selected, check that tiles flags and either move to
    // it, take to it, or do nothing.
    else if (auto& selectedPiece {board.GetSelectedPiece()}; !selectedPiece.IsNull())
    {
        Tile& targetTile {board[sqr.z, sqr.x]};
        if(targetTile.flags & Tile::POSSIBLEMOVE)
        {
            board.MovePieceToTile(selectedPiece, targetTile, appData->assets);
            endTurn = true;
        }
        else if (targetTile.flags & Tile::POSSIBLETAKE)
        {
            board.TakeToTile(selectedPiece, targetTile, appData->assets);
            endTurn = !board.SelectedPieceCanTake();
        }
    }
    return endTurn;
}

inline void UpdateCurrentBoard(AppData* appData)
{
    if (appData->input.clickFlags & PlayerInput::LCLICK)
    {
        if (ResolveLeftClick(appData))
            appData->board.EndTurn(&appData->assets);
    }
    else if (appData->input.clickFlags & PlayerInput::RCLICK)
    {
        appData->board.DeSelectPiece(&appData->assets);
    }
    appData->input.clickFlags = 0;
}

inline void UpdateGame(AppData* appData)
{
    UpdateCamera(appData);
    UpdateCurrentBoard(appData);
}

#endif