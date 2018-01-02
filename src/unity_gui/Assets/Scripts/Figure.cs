using UnityEngine;

public class Figure
{
    private FigureType ftype;
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
                ftype = FigureType.PAWN;
                player = Player.BLACK;
                break;
            case 'P':
                ftype = FigureType.PAWN;
                player = Player.WHITE;
                break;
            case 'b':
                ftype = FigureType.BISHOP;
                player = Player.BLACK;
                break;
            case 'B':
                ftype = FigureType.BISHOP;
                player = Player.WHITE;
                break;
            case 'n':
                ftype = FigureType.KNIGHT;
                player = Player.BLACK;
                break;
            case 'N':
                ftype = FigureType.KNIGHT;
                player = Player.WHITE;
                break;
            case 'r':
                ftype = FigureType.ROOK;
                player = Player.BLACK;
                break;
            case 'R':
                ftype = FigureType.ROOK;
                player = Player.WHITE;
                break;
            case 'q':
                ftype = FigureType.QUEEN;
                player = Player.BLACK;
                break;
            case 'Q':
                ftype = FigureType.QUEEN;
                player = Player.WHITE;
                break;
            case 'k':
                ftype = FigureType.KING;
                player = Player.BLACK;
                break;
            case 'K':
                ftype = FigureType.KING;
                player = Player.WHITE;
                break;
            default:
                ftype = FigureType.EMPTY;
                player = Player.NONE;
                break;
        }
    }

    public Figure()
    {
        ftype = FigureType.EMPTY;
        player = Player.NONE;
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

    public FigureType getFigureType()
    {
        if (ftype == null || ftype == FigureType.EMPTY)
        {
            return FigureType.EMPTY;
        }
        return ftype;
    }

    public void setFigureType(FigureType aType)
    {
        if (ftype != aType)
        {
            ftype = aType;
        }
    }
}