using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System;

public class PathGenerator : MonoBehaviour {
    
    private string bestMove;
    
    private string[] fenString = { "", "", "", "", "", "", ""};

    private ChessBoard board;

    private GameObject[] pathObjects;

    private GameObject startPlane;
    private GameObject endPlane;

    public GameObject parent;

    public GuiManager UiManager;

    public Material material;

    public Material depthMaskMaterial;

    public Material depthMaskFigureMaterial;

    [Tooltip("GameObject with script to animate Figure.")]
    public JumpAnimator jumpingFigure;

    public GameObject[] depthMaskFigures;

    public GameObject[] figureList;

    private bool bestMoveIsRunning = false;

    private List<GameObject> depthMaskList = new List<GameObject>();

    private List<GameObject> depthMaskFigureMaterialList = new List<GameObject>();

    private GameObject currentFigure;

    private GameObject[] doubleFigures;

    protected const float xBorder = 11;

    protected const float zBorder = 14;

    protected const float height = 3.5F;

    protected const float width = 3.5F;

    protected const float depth = 22F;

    public void receiveFen(string fen)
    {

        fenString = fen.Split(' ');

        board = new ChessBoard(fenString);

        bestMove = fenString[fenString.Length - 1];

        int line = 0;
        int col = getColumn(bestMove.Substring(0, 1));

        Int32.TryParse(bestMove.Substring(1, 1), out line);

        UiManager.receiveFenString(fenString, board);

        generateDepthMaskFigures();
    }
    
    public void generateBestMove()
    {
        resetDepthMaskFigureMaterialList();
        Destroy(startPlane);
        Destroy(endPlane);
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

                if (getFigure(8 - line1, col1 - 1).getType() != Figure.FigureType.EMPTY)
                {
                    Vector3 startPosition = new Vector3(calculateXPosition(col1), depth, calculateZPosition(line1));
                    Vector3 endPosition = new Vector3(calculateXPosition(col2), depth, calculateZPosition(line2));

                    instantiateSingleFigure(getFigure(8 - line1, col1 - 1), line1, col1);
                    jumpingFigure.setFigure(currentFigure, startPosition, endPosition, diffColumn, diffLine, getFigure(8 - line1, col1 - 1).getType());

                    depthMaskFigureMaterialList.Add(getDepthMaskObject(8 - line1, col1 - 1));
                    if (getFigure(8 - line2, col2 - 1).getType() != Figure.FigureType.EMPTY)
                    {
                        depthMaskFigureMaterialList.Add(getDepthMaskObject(8 - line2, col2 - 1));
                    }
                    setShader(depthMaskFigureMaterial);

                    startPlane = GameObject.CreatePrimitive(PrimitiveType.Plane);
                    startPlane.transform.position = new Vector3(calculateXPosition(col1), depth, calculateZPosition(line1));
                    startPlane.transform.localScale = new Vector3(width, 1, calculateHeight(line1));
                    startPlane.transform.parent = parent.transform;
                    startPlane.GetComponent<MeshRenderer>().material = material;

                    endPlane = GameObject.CreatePrimitive(PrimitiveType.Plane);
                    endPlane.transform.position = new Vector3(calculateXPosition(col2), depth, calculateZPosition(line2));
                    endPlane.transform.localScale = new Vector3(width, 1, calculateHeight(line2));
                    endPlane.transform.parent = parent.transform;
                    endPlane.GetComponent<MeshRenderer>().material = material;

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

        for (int line = 0; line < 8; line++)
        {
            for (int col = 0; col < 8; col++)
            {
                figure = board.getFigure(line, col);
                if (figure.getType() != Figure.FigureType.EMPTY)
                {
                    Vector3 scale = figure.getType() != Figure.FigureType.PAWN ? new Vector3(275, 275, 275) : new Vector3(250, 250, 250);
                    GameObject obj = Instantiate(depthMaskFigures[(int)figure.getType()], new Vector3(calculateXPosition(1 + col), depth, calculateZPosition(8 - line)), getQuaternion(figure));
                    obj.transform.parent = GameObject.Find("Figure").transform;
                    obj.transform.localScale = scale;
                    depthMaskList.Add(obj);
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
                if (figure.getType() != Figure.FigureType.EMPTY)
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

    public void generatePath(string fen)
    {

        
        Destroy(startPlane);
        Destroy(endPlane);

        if (bestMove != null)
        {
            if (bestMove != "(none)")
            {
                int col1 = getColumn(bestMove.Substring(0, 1));
                int col2 = getColumn(bestMove.Substring(2, 1));

                int diffColumn = col1 > col2 ? col1 - col2 : col2 - col1;

                int line1 = getLine(bestMove.Substring(1, 1));
                int line2 = getLine(bestMove.Substring(3, 1));
                
                int diffLine = line1 > line2 ? line1 - line2 : line2 - line1;

                if (getFigure(line1 - 1, col1 - 1).getType() != Figure.FigureType.EMPTY)
                {
                    //aFigure.SetEndPosition(new Vector3(12 + 38 * col2, 20F, 14 + 38 * line2));
                    //aFigure.InstantiateFigure(getFigure(line1 - 1, col1 - 1).getType(), new Vector3(12 + 38 * col1, 20F, 14 + 38 * line1), board.GetPlayer());

                    startPlane = GameObject.CreatePrimitive(PrimitiveType.Plane);
                    startPlane.transform.position = new Vector3(12 + 38 * col1, 21.6F, 14 + 38 * line1);
                    startPlane.transform.localScale = new Vector3(3.8F, 1, 3.8F);
                    startPlane.transform.parent = parent.transform;
                    startPlane.GetComponent<MeshRenderer>().material = material;

                    endPlane = GameObject.CreatePrimitive(PrimitiveType.Plane);
                    endPlane.transform.position = new Vector3(12 + 38 * col2, 21.6F, 14 + 38 * line2);
                    endPlane.transform.localScale = new Vector3(3.8F, 1, 3.8F);
                    endPlane.transform.parent = parent.transform;
                    endPlane.GetComponent<MeshRenderer>().material = material;

                    Vector3 offset = endPlane.transform.localPosition - startPlane.transform.localPosition;
                    float pathLength = Mathf.Sqrt(Mathf.Pow(offset.x, 2) + Mathf.Pow(0, 2) + Mathf.Pow(offset.z, 2));

                    //aFigure.SetJumpDuration(pathLength, (7F / 30F));
                }
                /*
                int meshCount;

                if ((diffColumn == 1 && diffLine == 2) || (diffColumn == 2 && diffLine == 1))
                {
                    meshCount = 4;
                }
                else
                {
                    meshCount = diffColumn > diffLine ? diffColumn : diffLine;
                    meshCount++;
                }

                int positionX = 12 + 38 * col1;
                int positionZ = 14 + 38 * line1;
                
                if (pathObjects != null)
                {
                    for (int mesh = 0; mesh < pathObjects.Length; mesh++)
                    {
                        Destroy(pathObjects[mesh]);
                    }

                }


                pathObjects = new GameObject[meshCount];

                for (int i = 0; i < meshCount; i++)
                {

                    if (i != 0)
                    {
                        if (diffColumn == diffLine)
                        {
                            positionX += Math.Sign(col2 - col1) * 38;
                            positionZ += Math.Sign(line2 - line1) * 38;
                        }
                        else if (diffColumn == 0)
                        {
                            positionZ += Math.Sign(line2 - line1) * 38;
                        }
                        else if (diffLine == 0)
                        {
                            positionX += Math.Sign(col2 - col1) * 38;
                        }
                        else
                        {
                            if (diffColumn > diffLine)
                            {
                                if (i == 3)
                                {
                                    positionZ += Math.Sign(line2 - line1) * 38;
                                }
                                else
                                {
                                    positionX += Math.Sign(col2 - col1) * 38;
                                }
                            }
                            else
                            {
                                if (i == 3)
                                {
                                    positionX += Math.Sign(col2 - col1) * 38;
                                }
                                else
                                {
                                    positionZ += Math.Sign(line2 - line1) * 38;
                                }
                            }
                        }
                    }

                    //if (diffLine != 0 && positionZ > 186 && positionZ < 224)
                    //{
                    //positionZ += 2;
                    //}

                    GameObject plane = GameObject.CreatePrimitive(PrimitiveType.Plane);
                    plane.transform.position = new Vector3(positionX, 20, positionZ);// Cube : 58;
                    plane.transform.localScale = new Vector3(3.8F, 1, 3.8F);// Cube: new Vector3(35, 70, 35)
                    plane.transform.parent = parent.transform;
                    plane.GetComponent<MeshRenderer>().material = material;

                    pathObjects[i] = plane;
                }*/
            }
            else
            {
                //Schachmatt
            }
        }
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
            return Quaternion.Euler(0, 180, 0);
        }

        return Quaternion.identity;
    }

    private void instantiateSingleFigure(Figure aFigure, int line, int col)
    {
        if (currentFigure != null)
        {
            currentFigure.SetActive(false);
        }
        currentFigure = figureList[(int)aFigure.getType()];
        currentFigure.SetActive(true);
        currentFigure.transform.SetPositionAndRotation(new Vector3(calculateXPosition(col), depth, calculateZPosition(line)), getQuaternion(aFigure));

    }
    
    private float calculateXPosition(int col)
    {
        return xBorder + col * 38;
    }

    private float calculateZPosition(int line)
    {
        float result = zBorder + line * 38;
        if (line > 4)
        {
            result += 1.5F;
        }
        return result;
    }

    private float calculateHeight(int line)
    {
        if (line == 5)
        {
            return 3.8F;
        }
        return height;
    }
}
