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
            Vector3 end = new Vector3(endPosition.x, figure.position.y, endPosition.z);

            if ((figure.position.x > startPosition.x - 0.01 && figure.position.x < startPosition.x + 0.01) &&
                (figure.position.z > startPosition.z - 0.01 && figure.position.z < startPosition.z + 0.01))
            {
                currentFigure.GetComponent<Rigidbody>().AddForce(new Vector3(0, 70, 0), ForceMode.Impulse);
            }

            Vector3 test = Vector3.MoveTowards(figure.position, end, duration * Time.deltaTime);
            figure.position = new Vector3(test.x, figure.position.y, test.z);
            
            if ((figure.position.x > endPosition.x - 0.01 && figure.position.x < endPosition.x + 0.01) &&
                (figure.position.z > endPosition.z - 0.01 && figure.position.z < endPosition.z + 0.01))
            {
                if (!isWaiting)
                {
                    //StartCoroutine(wait(1.0F));
                }
            }
        }
    }

    public void setFigure(GameObject aFigure, Vector3 aStartPosition, Vector3 anEndPosition, int diffX, int diffZ, Figure.FigureType aFigureType)
    {
        currentFigure = aFigure;
        startPosition = aStartPosition;
        endPosition = anEndPosition;

        float factor = aFigureType != Figure.FigureType.KNIGHT ? 25F : 12.5F;

        offset = new Vector3(endPosition.x - startPosition.x, endPosition.y - startPosition.y, endPosition.z - startPosition.z);
        duration = Mathf.Sqrt(Mathf.Pow( diffX * factor, 2) + Mathf.Pow(diffZ * factor, 2));
    }

    public void toggleAnimation()
    {
        showAnimation = !showAnimation;
    }

    public void SetJumpDuration(float distance, float factor)
    {

    }

    IEnumerator wait(float seconds)
    {
        isWaiting = true;
        yield return new WaitForSeconds(seconds);

        currentFigure.transform.position = startPosition;
        isWaiting = false;
    }
}
