using System.Collections;
using System.Collections.Generic;
using UnityEngine.UI;
using UnityEngine;
using System;

public class GuiManager : MonoBehaviour {

    private bool isDisplayed = false;

    private string[] fenString;
    private ChessBoard board;

    private Figure.Player currentPlayer;
    
    public GameObject BestMove;
    public GameObject EnPassant;
    public GameObject CastlingKingside;
    public GameObject CastlingQueenside;

    public Text CurrentPlayerText;
    public Text RoundText;
    public Text RemisText;
    public Text BestMoveText;
    public Text EnPassantText;

    public Image currentPlayerIcon;

    public Image bestMoveImage;
    public Sprite[] iconImages;

    public Button[] pawns;
    public Button[] figures;

    public Color enabledColor;
    public Color disabledColor;
    public Color focusedColor;

    void Start() {
        currentPlayer = Figure.Player.NONE;
        BestMove.SetActive(false);
        EnPassant.SetActive(false);
        CastlingKingside.SetActive(false);
        CastlingQueenside.SetActive(false);
        disableButtons();
    }
    
    public void receiveFenString(string[] aFenString, ChessBoard aBoard)
    {
        disableButtons();
        board = aBoard;
        fenString = aFenString;
        setCurrentPlayer(aFenString[1]);
        
        setBestMove(aFenString[6]);
        setCastling(aFenString[2]);
        setEnPassant(aFenString[3]);
        enableButtons();
        setRemis(aFenString[4]);
        setRound(aFenString[5]);
    }

    public void togglePanel (Animator aAnimator)
    {
        isDisplayed = !isDisplayed;
        aAnimator.SetBool("IsDisplayed", isDisplayed);
    }

    private void setCurrentPlayer(string aPlayer)
    {
        if (aPlayer == "w")
        {
            currentPlayer = Figure.Player.WHITE;
            CurrentPlayerText.text = "White";
            currentPlayerIcon.color = Color.white;
        }
        else if (aPlayer == "b")
        {
            currentPlayer = Figure.Player.BLACK;
            CurrentPlayerText.text = "Black";
            currentPlayerIcon.color = Color.black;
        }
        else
        {
            currentPlayer = Figure.Player.NONE;
            CurrentPlayerText.text = "No Player";
        }
    }

    private void setBestMove(string aBestMove)
    {
        BestMove.SetActive(true);
        if (aBestMove == "(none)")
        {
            BestMoveText.text = "No Move";
        }
        else
        {
            BestMoveText.text = aBestMove.Substring(0, 2).ToUpper() + " - " + aBestMove.Substring(2, 2).ToUpper();
            int figureType = (int) board.getFigure(8 - getLine(aBestMove.Substring(1, 1)), getColumn(aBestMove.Substring(0, 1)) - 1).getType();
            bestMoveImage.sprite = iconImages[figureType];
        }
    }

    private void setRound(string aRound)
    {
        RoundText.text = aRound;
    }

    private void setRemis(string aRemis)
    {
        int remis = Int32.Parse(aRemis);
        remis = 50 - remis;
        RemisText.text = remis.ToString();
    }

    private void setEnPassant(string enPassant)
    {
        if (enPassant != "-")
        {
            EnPassant.SetActive(true);
            EnPassantText.text = enPassant.ToUpper();
        }
        else
        {
            EnPassant.SetActive(false);
        }
    }

    private void setCastling(string aCastling)
    {
        int king = 0;
        int queen = 1;

        if (fenString[1] == "b")
        {
            king = 2;
            queen = 3;
        }

        if (aCastling.Substring(king, 1) != "-")
        {
            CastlingKingside.SetActive(true);
        }
        else
        {
            CastlingKingside.SetActive(false);
        }

        if (aCastling.Substring(queen, 1) != "-")
        {
            CastlingQueenside.SetActive(true);
        }
        else
        {
            CastlingQueenside.SetActive(false);
        }

    }

    private void enableButtons()
    {
        int pawnIndex = 0;
        List<Figure> figureList = board.getFiguresOfPlayer(currentPlayer);
        foreach (Figure aFigure in figureList)
        {
            Figure.FigureType aType = aFigure.getType();
            Button but;

            if (Figure.FigureType.PAWN == aType)
            {
                but = pawns[pawnIndex];
                pawnIndex++;
            }
            else
            {
                switch (aType)
                {
                    case Figure.FigureType.BISHOP:
                        but = figures[0].enabled ? figures[1] : figures[0];
                        break;

                    case Figure.FigureType.KNIGHT:
                        but = figures[2].enabled ? figures[3] : figures[2];
                        break;

                    case Figure.FigureType.ROOK:
                        but = figures[4].enabled ? figures[5] : figures[4];
                        break;

                    case Figure.FigureType.QUEEN:
                        but = figures[6];
                        break;

                    default:
                        but = figures[7];
                        break;
                }
            }

            but.enabled = true;
            but.GetComponent<Image>().color = enabledColor;
        }
    }

    private void disableButtons()
    {
        foreach (Button button in pawns)
        {
            button.enabled = false;
            button.GetComponent<Image>().color = disabledColor;
        }

        foreach (Button button in figures)
        {
            button.enabled = false;
            button.GetComponent<Image>().color = disabledColor;
        }
    }

    public void onButtonClicked(Button aButton)
    {
        for (int button = 0; button < figures.Length; button++)
        {
            if (figures[button].enabled)
            {
                figures[button].GetComponent<Image>().color = enabledColor;
            }
            if (pawns[button].enabled)
            {
                pawns[button].GetComponent<Image>().color = enabledColor;
            }
        }
        aButton.GetComponent<Image>().color = focusedColor;
    }

    private int getColumn(string move)
    {
        switch (move)
        {
            case "a":
                return 1;
            case "b":
                return 2;
            case "c":
                return 3;
            case "d":
                return 4;
            case "e":
                return 5;
            case "f":
                return 6;
            case "g":
                return 7;
            case "h":
                return 8;
            default:
                return 0;
        }
    }

    private int getLine(string aLine)
    {
        int line = 0;
        Int32.TryParse(aLine, out line);
        return line;
    }
}
