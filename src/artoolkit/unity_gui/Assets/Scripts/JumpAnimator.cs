using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class JumpAnimator : MonoBehaviour {
    
    private Vector3 startPosition;
    private Vector3 endPosition;
    
    private bool showAnimation = false;
    
    private GameObject currentFigure;

    public GameObject[] figureList;

    public GameObject parent;
            
    public float duration = 48F;
    
    private Figure.Player player;

    private Vector3 offset = Vector3.zero;

    private bool isWaiting = false;

    // Use this for initialization
    void Start () {
        currentFigure = new GameObject();
    }
    	
	// Update is called once per frame
	void Update () {
        if (showAnimation)
        {
            Transform figure = currentFigure.transform;
            Vector3 end = new Vector3(endPosition.x, endPosition.y, figure.position.z);

            if ((figure.position.x > startPosition.x - 0.0001 && figure.position.x < startPosition.x + 0.0001) &&
                (figure.position.y > startPosition.y - 0.0001 && figure.position.y < startPosition.y + 0.0001))
            {
                currentFigure.GetComponent<Rigidbody>().AddForce(new Vector3(0, 0, -0.07F), ForceMode.Impulse);
            }
            
            Vector3 test = Vector3.MoveTowards(figure.position, end, duration * Time.deltaTime * 0.09F);
            figure.position = new Vector3(test.x, test.y, figure.position.z);
            
            if ((figure.position.x > endPosition.x - 0.0001 && figure.position.x < endPosition.x + 0.0001) &&
                (figure.position.y > endPosition.y - 0.0001 && figure.position.y < endPosition.y + 0.0001))
            {
                Debug.LogError("At the end");
                if (!isWaiting)
                {
                    //StartCoroutine(wait(1.0F));
                }
            }
        }
    }

    public void setFigure(GameObject aFigure, Vector3 aStartPosition, Vector3 anEndPosition, int diffX, int diffY, Figure.FigureType aFigureType)
    {
        currentFigure = aFigure;
        startPosition = aStartPosition;
        endPosition = anEndPosition;

        float factor = aFigureType != Figure.FigureType.KNIGHT ? 0.25F : 0.125F;

        offset = new Vector3(endPosition.x - startPosition.x, endPosition.y - startPosition.y, endPosition.z - startPosition.z);
        duration = Mathf.Sqrt(Mathf.Pow( diffX * factor, 2) + Mathf.Pow(diffY * factor, 2));
        Debug.LogError("offset.x: " + offset.x);
        Debug.LogError("offset.y: " + offset.y);
        Debug.LogError("offset.z: " + offset.z);
    }

    public void toggleAnimation()
    {
        showAnimation = !showAnimation;
    }

    IEnumerator wait(float seconds)
    {
        isWaiting = true;
        yield return new WaitForSeconds(seconds);

        currentFigure.transform.position = startPosition;
        isWaiting = false;
    }
}
