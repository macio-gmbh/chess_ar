using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System;

public class PathGenerator : MonoBehaviour
{

    private string bestMove;

    private string[] fenString = { "", "", "", "", "", "", "" };

    private ChessBoard board;

    private GameObject[] pathObjects;

    private GameObject startPlane;
    private GameObject endPlane;

    public GameObject parent;

    public GameObject figureGameObject;

    public GuiManager UiManager;

    public Material material;

    public Material materialStrike;

    public Material depthMaskMaterial;

    public Material depthMaskFigureMaterial;

    [Tooltip("GameObject with script to animate Figure.")]
    public JumpAnimator jumpingFigure;

    public GameObject[] depthMaskFigures;

    public GameObject[] figureList;

    public Material whiteMaterial;

    public Material blackMaterial;

    private bool bestMoveIsRunning = false;

    private List<GameObject> depthMaskList = new List<GameObject>();

    private List<GameObject> depthMaskFigureMaterialList = new List<GameObject>();

    private GameObject currentFigure;

    private GameObject[] doubleFigures;

    private string currentPlayer;

    private string lastFen = "";

    protected const float xBorder = 0.009F;

    protected const float yBorder = 0.010F;

    protected const float height = 0.035F;

    protected const float width = 0.035F;

    protected const float depth = 0;

    public void receiveFen(string fen)
    {
        if (bestMoveIsRunning)
        {
            generateBestMove();
        }

        fenString = fen.Split(' ');

        board = new ChessBoard(fenString);

        bestMove = fenString[fenString.Length - 1];

        currentPlayer = fenString[1];

        int line = 0;
        int col = getColumn(bestMove.Substring(0, 1));

        Int32.TryParse(bestMove.Substring(1, 1), out line);

        UiManager.receiveFenString(fenString, board);

        generateDepthMaskFigures();
        //generateDepthMaskFigureMaterialList(bestMove);
    }

    public void receiveError(string errorString)
    {
        UiManager.setError(errorString);
    }

    public void generateBestMove()
    {
        Destroy(startPlane);
        Destroy(endPlane);
        jumpingFigure.toggleAnimation();
        if (!bestMoveIsRunning)
        {
            bestMoveIsRunning = true;
            if (bestMove != null || bestMove != "(none)")
            {
                int col1 = getColumn(bestMove.Substring(0, 1));
                int col2 = getColumn(bestMove.Substring(2, 1));

                int diffColumn = col1 > col2 ? col1 - col2 : col2 - col1;

                int line1 = getLine(bestMove.Substring(1, 1));
                int line2 = getLine(bestMove.Substring(3, 1));

                int diffLine = line1 > line2 ? line1 - line2 : line2 - line1;

                if (getFigure(8 - line1, col1 - 1).getFigureType() != Figure.FigureType.EMPTY)
                {
                    Vector3 startPosition = new Vector3(calculateXPosition(col1), calculateYPosition(line1), depth);
                    Vector3 endPosition = new Vector3(calculateXPosition(col2), calculateYPosition(line2), depth);

                    instantiateSingleFigure(getFigure(8 - line1, col1 - 1), startPosition);
                    jumpingFigure.setFigure(currentFigure, startPosition, endPosition, diffColumn, diffLine, getFigure(8 - line1, col1 - 1).getFigureType());

                    startPlane = GameObject.CreatePrimitive(PrimitiveType.Plane);
                    startPlane.transform.position = startPosition;
                    startPlane.transform.localScale = new Vector3(width * 0.1F, 1, calculateHeight(line1) * 0.1F);
                    startPlane.transform.parent = parent.transform;
                    startPlane.transform.SetPositionAndRotation(startPlane.transform.position, Quaternion.Euler(-90, 0, 0));
                    startPlane.layer = 10;

                    endPlane = GameObject.CreatePrimitive(PrimitiveType.Plane);
                    endPlane.transform.position = endPosition;
                    endPlane.transform.localScale = new Vector3(width * 0.1F, 1, calculateHeight(line2) * 0.1F);
                    endPlane.transform.parent = parent.transform;
                    endPlane.transform.SetPositionAndRotation(endPlane.transform.position, Quaternion.Euler(-90, 0, 0));
                    endPlane.layer = 10;

                    if (getFigure(8 - line2, col2 - 1).getFigureType() != Figure.FigureType.EMPTY)
                    {
                        endPlane.GetComponent<MeshRenderer>().material = materialStrike;
                        startPlane.GetComponent<MeshRenderer>().material = materialStrike;
                    }
                    else
                    {
                        endPlane.GetComponent<MeshRenderer>().material = material;
                        startPlane.GetComponent<MeshRenderer>().material = material;
                    }

                    Vector3 offset = endPlane.transform.localPosition - startPlane.transform.localPosition;
                    float pathLength = Mathf.Sqrt(Mathf.Pow(offset.x, 2) + Mathf.Pow(0, 2) + Mathf.Pow(offset.z, 2));

                }
            }
        }
        else
        {
            currentFigure.SetActive(false);
            bestMoveIsRunning = false;
        }
    }

    private void generateDepthMaskFigures()
    {
        while (depthMaskList.Count != 0)
        {
            GameObject lastInstance = depthMaskList[depthMaskList.Count - 1];
            depthMaskList.RemoveAt(depthMaskList.Count - 1);
            Destroy(lastInstance);
        }

        Figure figure;
        
        int colTarget = getColumn(bestMove.Substring(2, 1)) - 1;
        
        int lineTarget = 8 - getLine(bestMove.Substring(3, 1));

        for (int line = 0; line < 8; line++)
        {
            for (int col = 0; col < 8; col++)
            {
                figure = board.getFigure(line, col);
                //Debug.Log("Line: " + line);
                //Debug.Log("Column: " + col);
                //Debug.Log(figure.getFigureType());
                if (figure.getFigureType() != Figure.FigureType.EMPTY)
                {
                    if (!(col == colTarget && line == lineTarget))
                    {
                        GameObject obj = Instantiate(depthMaskFigures[(int)figure.getFigureType()], new Vector3(calculateXPosition(1 + col), calculateYPosition(8 - line), depth), getQuaternion(figure));
                        obj.transform.SetParent(figureGameObject.transform);
                        obj.layer = 10;
                        depthMaskList.Add(obj);
                    }
                }
            }
        }
    }

    private GameObject getDepthMaskObject(int line, int col)
    {
        int index = 0;
        Figure figure;
        for (int i = 0; i < line + 1; i++)
        {
            int colLength = (i == line) ? col + 1 : 8;

            for (int j = 0; j < colLength; j++)
            {
                figure = board.getFigure(i, j);
                if (figure.getFigureType() != Figure.FigureType.EMPTY)
                {
                    index++;
                }
            }
        }
        return depthMaskList[index - 1];
    }

    private void setShader(Material shaderMaterial)
    {
        foreach (GameObject obj in depthMaskFigureMaterialList)
        {
            MeshRenderer[] meshes = obj.GetComponentsInChildren<MeshRenderer>();
            foreach (MeshRenderer mesh in meshes)
            {
                mesh.material = shaderMaterial;
            }
        }
    }

    public void resetDepthMaskFigureMaterialList()
    {
        setShader(depthMaskMaterial);
        depthMaskFigureMaterialList = new List<GameObject>();
    }

    public void generateDepthMaskFigureMaterialList(string BestMove)
    {
        depthMaskFigureMaterialList = new List<GameObject>();

        int col1 = getColumn(bestMove.Substring(0, 1));
        int col2 = getColumn(bestMove.Substring(2, 1));
        int line1 = getLine(bestMove.Substring(1, 1));
        int line2 = getLine(bestMove.Substring(3, 1));

        depthMaskFigureMaterialList.Add(getDepthMaskObject(8 - line1, col1 - 1));
        if (getFigure(8 - line2, col2 - 1).getFigureType() != Figure.FigureType.EMPTY)
        {
            depthMaskFigureMaterialList.Add(getDepthMaskObject(8 - line2, col2 - 1));
        }
        setShader(depthMaskFigureMaterial);
    }

    private Figure getFigure(int line, int col)
    {
        return board.getFigure(line, col);
    }

    Figure.FigureType mapStringToFigure(Char fen)
    {
        switch (fen)
        {
            case 'k':
            case 'K':
                return Figure.FigureType.KING;
            case 'q':
            case 'Q':
                return Figure.FigureType.QUEEN;
            case 'b':
            case 'B':
                return Figure.FigureType.BISHOP;
            case 'n':
            case 'N':
                return Figure.FigureType.KNIGHT;
            case 'r':
            case 'R':
                return Figure.FigureType.ROOK;
            case 'p':
            case 'P':
                return Figure.FigureType.PAWN;
            default:
                return Figure.FigureType.EMPTY;
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

    private Quaternion getQuaternion(Figure aFigure)
    {
        if (aFigure.getPlayer() == Figure.Player.BLACK)
        {
            return Quaternion.Euler(90, 180, 0);
        }

        return Quaternion.Euler(-90, 0, 0);
    }

    private void instantiateSingleFigure(Figure aFigure, Vector3 position)
    {
        if (currentFigure != null)
        {
            Destroy(currentFigure);
        }
        currentFigure = Instantiate(figureList[(int)aFigure.getFigureType()]);
        currentFigure.transform.SetPositionAndRotation(position, getQuaternion(aFigure));
        currentFigure.transform.SetParent(figureGameObject.transform);
        currentFigure.layer = 10;

        Material[] mats;
        if (currentPlayer == "w")
        {
            mats = new Material[] { whiteMaterial };
        }
        else
        {
            mats = new Material[] { blackMaterial };
        }

        MeshRenderer[] meshes = currentFigure.GetComponentsInChildren<MeshRenderer>();
        foreach (MeshRenderer mesh in meshes)
        {
            mesh.materials = mats;
        }

        /*
        if (currentFigure != null)
        {
            currentFigure.SetActive(false);
        }
        currentFigure = figureList[(int)aFigure.getFigureType()];
        currentFigure.SetActive(true);
        currentFigure.transform.SetPositionAndRotation(new Vector3(calculateXPosition(col), depth, calculateZPosition(line)), getQuaternion(aFigure));
        */
    }

    private float calculateXPosition(int col)
    {
        return xBorder + (col * 0.0365F);
    }

    private float calculateYPosition(int line)
    {
        float result = yBorder + line * 0.0365F;
        if (line > 4)
        {
            result += 0.0015F;
        }
        return result;
    }

    private float calculateHeight(int line)
    {
        if (line == 5)
        {
            return 0.038F;
        }
        return height;
    }
}
