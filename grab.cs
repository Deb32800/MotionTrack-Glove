using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.IO.Ports;
using System;

public class grab : MonoBehaviour
{

    float bone_max_rotation = -70;

    private GameObject[] left_index;
    private GameObject[] left_pinky;
    private GameObject[] left_ring;
    private GameObject[] left_middle;
    private GameObject[] left_thumb;
    private GameObject left_hand;
    private GameObject sphere;

    float left_index_degree = 0;
    float left_middle_degree = 0;
    float left_ring_degree = 0;
    float left_thumb_degree = 0;
    float left_pinky_degree = 0;

    // {translatex, translatey, translatez, rotatex, rotatey, rotatez}
    float[] left_hand_trans = new float[] { 0.0f, 0.0f, 0.0f};
    float[] left_hand_rot = new float[] { 0.0f, 0.0f, 0.0f };

    float val;
    private string COM_PORT = "COM6";
    SerialPort sp = new SerialPort(COM_PORT, 9600);
    public string SerialOutput;
    public string[] SerialOutputs;

    public GameObject cube;
    public GameObject newCube;
    public List<GameObject> listofCubes;

    // Start is called before the first frame update
    void Start()
    {


        left_index = GameObject.FindGameObjectsWithTag("left_index");
        left_pinky = GameObject.FindGameObjectsWithTag("left_pinky");
        left_ring = GameObject.FindGameObjectsWithTag("left_ring");
        left_middle = GameObject.FindGameObjectsWithTag("left_middle");
        left_thumb = GameObject.FindGameObjectsWithTag("left_thumb");
        left_hand = GameObject.FindWithTag("left_hand");
        //sphere = GameObject.FindWithTag("sphere");

        /* Test rotations
        rotate_finger(left_index, degree);
        rotate_finger(left_pinky, degree);
        rotate_finger(left_ring, degree);
        rotate_finger(left_middle, degree);
        rotate_finger(left_thumb, degree/2);

        rotate_hand(left_hand, 0,0,90);
        

        // Test translations
        translate_hand(left_hand, 1, 1, 1);
        grab_object(sphere, left_hand);
        */

        sp.Open();
        sp.ReadTimeout = 5000;


    }

        // Update is called once per frame
        void Update()
    {
        /*
        translate_hand(left_hand, 0, 0, 1);
        drop_object(sphere);
        */
        read_and_process_serial();
        processCubes();

    }

    // Finger
    void rotate_finger(GameObject[] bones, float rotation_value)
    {
        foreach (GameObject bone in bones)
            bone.transform.Rotate(0, 0, rotation_value* bone_max_rotation / 180);
    }

    // Hand rotation about 3 axis
    void rotate_hand(GameObject hand, float x_rotation_value, float y_rotation_value, float z_rotation_value)
    {
        hand.transform.Rotate(x_rotation_value, y_rotation_value, z_rotation_value);
    }

    // Hand translation about 3 axis
    void translate_hand(GameObject hand, float x_translation_value, float y_translation_value, float z_translation_value)
    {
        hand.transform.Translate(x_translation_value, y_translation_value, z_translation_value);
    }

    // Move object to grab to Hand's position and follow the hand
    void grab_object(GameObject object_to_grab, GameObject Hand)
    {
        object_to_grab.transform.position = Hand.transform.position;
        object_to_grab.transform.parent = Hand.transform;
        
    }

    // Remove the objects parent
    void drop_object(GameObject object_to_drop)
    {
        object_to_drop.transform.parent = null;
    }

    void read_and_process_serial()
    {
        SerialOutput = sp.ReadLine();
        print(SerialOutput);

        SerialOutputs = SerialOutput.Split(':');

        val = float.Parse(SerialOutputs[2]);

        /*
        if (SerialOutputs[1] == "Index")
        {
            rotate_finger(left_index, val- left_index_degree);
            left_index_degree = val;
        }
        else if (SerialOutputs[1] == "Middle")
        {
            rotate_finger(left_middle, val);
        }
        else if (SerialOutputs[1] == "Ring")
        {
            rotate_finger(left_ring, val);
        }
        else if (SerialOutputs[1] == "Thumb")
        {
            rotate_finger(left_thumb, val / 2);
        }
        else if (SerialOutputs[1] == "Pinky")
        {
            rotate_finger(left_pinky, val);
        }
        */
        
        if (SerialOutputs[0] == "Left")
        {
            switch (SerialOutputs[1])
            {
                case "Index":
                    rotate_finger(left_index, val - left_index_degree);
                    left_index_degree = val;
                    
                    break;

                case "Middle":
                    rotate_finger(left_middle, val- left_middle_degree);
                    left_middle_degree = val;
                    
                    break;

                case "Ring":
                    rotate_finger(left_ring, val- left_ring_degree);
                    left_ring_degree = val;
                    
                    break;
                case "Pinky":
                    rotate_finger(left_pinky, val- left_pinky_degree);
                    left_pinky_degree = val;
                    break;
                case "Thumb":
                    rotate_finger(left_thumb, val- left_thumb_degree);
                    left_thumb_degree = val;
                    break;
                case "RotateX":
                    rotate_hand(left_hand, val- left_hand_rot[0], 0, 0);
                    left_hand_rot[0]=val;
                    break;
                case "RotateY":
                    if (Math.Abs(val) < 1200)
                    {
                        val = 0;
                    }
                    rotate_hand(left_hand, 0,  0, val/700);
                    left_hand_rot[1]=val/700;
                    break;
                case "RotateZ":
                    if (Math.Abs(val) < 1200)
                    {
                        val = 0;
                    }
                    rotate_hand(left_hand, -1*val / 400,0, 0);
                    left_hand_rot[2] = -1 * val / 400;
                    break;
                case "TranslateX":
                    if (Math.Abs(val) < 600)
                    {
                        break;
                    }
                    val = val / 3000;
                    translate_hand(left_hand, val, 0, 0);
                    //left_hand_trans[0]=val;
                    break;
                case "TranslateY":
                    translate_hand(left_hand, 0, val- left_hand_trans[1], 0);
                    left_hand_trans[1]=val;
                    break;
                case "TranslateZ":
                    if (Math.Abs(val) < 1200)
                    {
                        val = 0;

                    }
                    else
                    {
                        val = 1;
                    }
                    translate_hand(left_hand,0, val - left_hand_trans[2],0);
                    left_hand_trans[2]=val;
                    break;

                default:
                    break;

            }
        }

        /*
        {             
            if (SerialOutputs[1] == "RotateX")
            {
                rotate_hand(left_hand, val, 0, 0);
            }
            else if (SerialOutputs[1] == "RotateY")
            {
                rotate_hand(left_hand, 0, val, 0);
            }
            else if (SerialOutputs[1] == "RotateZ")
            {
                rotate_hand(left_hand, 0, 0, val);
            }
            else if (SerialOutputs[1] == "TranslateX")
            {
                translate_hand(left_hand, val, 0, 0);
            }
            else if (SerialOutputs[1] == "TranslateY")
            {
                translate_hand(left_hand, 0, val, 0);
            }
            else if (SerialOutputs[1] == "TranslateZ")
            {
                translate_hand(left_hand, 0, 0, val);
            }
        }
        */
                   
    }

    void createCube(int x,int y,int z)
    {
        newCube=Instantiate(cube, new Vector3(x,y,z), Quaternion.identity);
        listofCubes.Add(newCube);
    }

    void processCubes()
    {
        if (left_ring_degree > 40)
        {
            createCube(-1, 2, 4);
        }
        if (left_index_degree > 40)
        {
            createCube(1, 2, 4);
        }
        if (left_middle_degree > 40)
        {
            createCube(0, 2, 4);
        }
        if (left_middle_degree > 60 & left_middle_degree > 60 & left_index_degree > 60 & left_ring_degree > 60 & left_thumb_degree >60)
        {
            destroyCubes();

        }

        if (left_hand_rot[2] > 20)
        {
            reverseGravity();
        }




    }

    void destroyCubes()
    {
        foreach (GameObject cube in listofCubes)
        {
            Destroy(cube);
        }
        listofCubes.Clear();

    }

    void reverseGravity()
    {
        foreach (GameObject cube in listofCubes)
        {
            cube.GetComponent<Rigidbody>().AddForce(0, 30F, 0);
        }
    }
}
