using UnityEngine;
using UnityEditor;

public class Figure : ScriptableObject
{
    private FigureType type;
    private Player player;

    public enum FigureType
    {
        KING = 0,
        QUEEN = 1,
        BISHOP = 2,
        KNIGHT = 3,
        ROOK = 4,
        PAWN = 5,
        EMPTY = 6
    }

    public enum Player
    {
        BLACK = 0,
        WHITE = 1,
        NONE  = 2
    }

    public Figure (char figureShortName)
    {
        switch (figureShortName)
        {
            case 'p':
                type = FigureType.PAWN;
                player = Player.BLACK;
                break;
            case 'P':
                type = FigureType.PAWN;
                player = Player.WHITE;
                break;
            case 'b':
                type = FigureType.BISHOP;
                player = Player.BLACK;
                break;
            case 'B':
                type = FigureType.BISHOP;
                player = Player.WHITE;
                break;
            case 'n':
                type = FigureType.KNIGHT;
                player = Player.BLACK;
                break;
            case 'N':
                type = FigureType.KNIGHT;
                player = Player.WHITE;
                break;
            case 'r':
                type = FigureType.ROOK;
                player = Player.BLACK;
                break;
            case 'R':
                type = FigureType.ROOK;
                player = Player.WHITE;
                break;
            case 'q':
                type = FigureType.QUEEN;
                player = Player.BLACK;
                break;
            case 'Q':
                type = FigureType.QUEEN;
                player = Player.WHITE;
                break;
            case 'k':
                type = FigureType.KING;
                player = Player.BLACK;
                break;
            case 'K':
                type = FigureType.KING;
                player = Player.WHITE;
                break;
            default:
                type = FigureType.EMPTY;
                player = Player.NONE;
                break;
        }
    }

    public Player getPlayer()
    {
        return player;
    }

    public void setPlayer(Player aPlayer)
    {
        if (player != aPlayer)
        {
            player = aPlayer;
        }
    }

    public FigureType getType()
    {
        return type;
    }

    public void setType(FigureType aType)
    {
        if (type != aType)
        {
            type = aType;
        }
    }
}