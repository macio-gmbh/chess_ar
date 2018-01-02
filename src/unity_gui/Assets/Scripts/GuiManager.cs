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

    public Color whiteColor;
    public Color blackColor;
    public Color enabledColor;
    public Color disabledColor;
    public Color focusedColor;

    public Receiver amqpReceiver;
    public Text errorLabel;
    public Text errorText;
    public Image errorImage;


    public GameObject DarkOverlay;
    public GameObject CheckmatedPanel;
    public GameObject WaitingPanel;
    public Image CheckmatedImage;
    public Text WinnerText;

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
        if (!isDisplayed)
        {
            amqpReceiver.update();
        }
    }

    public void setCurrentPlayer(string aPlayer)
    {
        if (aPlayer == "w")
        {
            currentPlayer = Figure.Player.WHITE;
            CurrentPlayerText.text = "White";
            currentPlayerIcon.color = whiteColor;
            CheckmatedImage.color = blackColor;
            WinnerText.text = "Black wins";
        }
        else if (aPlayer == "b")
        {
            currentPlayer = Figure.Player.BLACK;
            CurrentPlayerText.text = "Black";
            currentPlayerIcon.color = blackColor;
            CheckmatedImage.color = whiteColor;
            WinnerText.text = "White wins";
        }
        else
        {
            currentPlayer = Figure.Player.NONE;
            CurrentPlayerText.text = "No Player";
            WinnerText.text = "No one has won";
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
            int figureType = (int) board.getFigure(8 - getLine(aBestMove.Substring(1, 1)), getColumn(aBestMove.Substring(0, 1)) - 1).getFigureType();
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
            Figure.FigureType aType = aFigure.getFigureType();
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

    private Figure.FigureType getFigureType(string figure)
    {
        switch (figure)
        {
            case "K":
                return Figure.FigureType.KING;
            case "Q":
                return Figure.FigureType.QUEEN;
            case "R":
                return Figure.FigureType.ROOK;
            case "N":
                return Figure.FigureType.KNIGHT;
            case "B":
                return Figure.FigureType.BISHOP;
            case "P":
                return Figure.FigureType.PAWN;
            default:
                return Figure.FigureType.EMPTY;

        }
    }

    public void setError(string errorString)
    {
        string[] errorArray = errorString.Split(' ');
        if (errorArray[0] != "CHECK")
        {
            if (errorArray[0] == "ERROR1")
            {
                string figure = errorArray[1];
                string field = errorArray[2];
                errorLabel.text = "Missing Figure:";
                errorText.text = field;
                errorImage.enabled = true;
                errorImage.sprite = iconImages[(int)getFigureType(figure)];
            }
            else if (errorArray[0] == "ERROR2")
            {
                errorLabel.text = "Invalid Board:";
                errorText.text = "Recalibrate the eye or reset all figures";
                errorImage.enabled = false;
            }
            else if (errorArray[0] == "ERROR3")
            {
                string figure = errorArray[1];
                string move = errorArray[2].ToUpper();
                errorLabel.text = "Invalid Move:";
                errorText.text = move;
                errorImage.enabled = true;
                errorImage.sprite = iconImages[(int) getFigureType(figure)];
            }
            else if (errorArray[0] == "ERROR4")
            {
                errorLabel.text = "Wrong Figure detected:";
                errorText.text = "Make sure there is one figure each field";
                errorImage.enabled = false;
            }
        }
        else
        {
            string player = (currentPlayer == Figure.Player.WHITE) ? ("White") : ((currentPlayer == Figure.Player.BLACK) ? "Black" : "None");
            errorLabel.text = "Check:";
            errorText.text = player + " king is checked";
            errorImage.enabled = true;
            errorImage.sprite = iconImages[(int)getFigureType("K")];
        }
    }

    public void enableDarkOverlay()
    {
        DarkOverlay.SetActive(true);
    }
    
    public void disableDarkOverlay()
    {
        DarkOverlay.SetActive(false);
    }
    
    public void enableCheckmatedPanel()
    {
        CheckmatedPanel.SetActive(true);
    }

    public void disableCheckmatedPanel()
    {
        CheckmatedPanel.SetActive(false);
    }

    public void enableWaitingPanel()
    {
        WaitingPanel.SetActive(true);
    }

    public void disableWaitingPanel()
    {
        WaitingPanel.SetActive(false);
    }
}
