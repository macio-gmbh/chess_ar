using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class MarkerScript : MonoBehaviour {
    
    public GameObject obj;
    public GameObject Figures;
    public Rigidbody[] rb;
    bool tracked = false;

	void OnMarkerFound(ARMarker marker)
    {
        Figures.SetActive(true);
    }

    void OnMarkerTracked(ARMarker marker)
    {
        Figures.SetActive(true);
        foreach (Rigidbody rigid in rb)
        {
            rigid.useGravity = true;
        }
    }

    void OnMarkerLost(ARMarker marker)
    {
        Figures.SetActive(false);
        foreach (Rigidbody rigid in rb)
        {
            rigid.useGravity = false;
        }
    }
}
