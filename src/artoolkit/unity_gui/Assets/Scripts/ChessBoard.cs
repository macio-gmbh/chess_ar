using UnityEngine;
using System;
using System.Collections.Generic;

public class ChessBoard
{
    private string[] fenString;
    private Figure[,] board;

    public ChessBoard (string[] aFenString)
    {
        fenString = aFenString;
        string[] fenSeperate = fenString[0].Split('/');

        board = new Figure[8, 8];

        for (int line = 0; line < fenSeperate.Length; line++)
        {
            string lineOfFigures = fenSeperate[line];
            int col = 0;
            for (int i = 0; i < lineOfFigures.Length; i++)
            {
                if (Char.IsNumber(lineOfFigures[i]))
                {
                    int emptyFields = (int) Char.GetNumericValue(lineOfFigures[i]);
                    for (int empty = 0; empty < emptyFields; empty++)
                    {
                        board[line, col + empty] = new Figure(lineOfFigures[i]);
                    }
                    col += emptyFields;
                }
                else
                {
                    board[line, col] = new Figure(lineOfFigures[i]);
                    col++;
                }
            }
        }
    }

    public Figure getFigure(int line, int col)
    {
        return board[line, col];
    }

    public void setFigure(int line, int col, Figure aFigure)
    {
        board[line, col] = aFigure;
    }

    public List<Figure> getFiguresOfPlayer(Figure.Player aPlayer)
    {
        List<Figure> figures = new List<Figure>();
        for (int line = 0; line < board.GetLength(0); line++) {
            for (int col = 0; col < board.GetLength(1); col++)
            {
                Figure aFigure = getFigure(line, col);
                if (aFigure != null)
                {
                    if (aFigure.getPlayer() == aPlayer)
                    {
                        figures.Add(aFigure);
                    }
                }
            }
        }
        return figures;
    }

    public Figure.Player GetPlayer()
    {
        if (fenString[1] == "w")
        {
            return Figure.Player.WHITE;
        }
        else if (fenString[1] == "b")
        {
            return Figure.Player.BLACK;
        }
        else
        {
            return Figure.Player.NONE;
        }
    }
}