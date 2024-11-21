using UnityEngine;
using WebSocketSharp; // Ensure the WebSocketSharp.dll is installed
using TMPro;          // For TextMeshPro UI (optional)

// Attach this script to an empty GameObject in the scene
public class ESP32WebSocketClient : MonoBehaviour
{
    [Header("WebSocket Configuration")]
    public string esp32WebSocketUrl = "ws://192.168.230.200:81"; // Replace with your ESP32 IP and WebSocket port

    [Header("UI Components")]
    public TextMeshProUGUI outputText; // Optional: Text element to display data

    private WebSocket webSocket;

    void Start()
    {
        // Initialize WebSocket
        webSocket = new WebSocket(esp32WebSocketUrl);

        // Set up WebSocket event handlers
        webSocket.OnOpen += OnWebSocketOpen;
        webSocket.OnMessage += OnWebSocketMessage;
        webSocket.OnError += OnWebSocketError;
        webSocket.OnClose += OnWebSocketClose;

        // Connect to the WebSocket server
        Debug.Log("Connecting to WebSocket...");
        webSocket.Connect();
    }

    private void OnWebSocketOpen(object sender, System.EventArgs e)
    {
        Debug.Log("WebSocket connected.");
        if (outputText != null)
        {
            outputText.text = "Connected to ESP32!";
        }
    }

    private void OnWebSocketMessage(object sender, MessageEventArgs e)
    {
        Debug.Log("Received message: " + e.Data);

        // Update UI with received data
        if (outputText != null)
        {
            outputText.text = e.Data;
        }

        // Parse JSON data (optional, depending on the ESP32 message format)
        // Example: {"accelX":123,"accelY":456,"accelZ":789}
        try
        {
            var sensorData = JsonUtility.FromJson<SensorData>(e.Data);
            Debug.Log($"AccelX: {sensorData.accelX}, AccelY: {sensorData.accelY}, AccelZ: {sensorData.accelZ}");
        }
        catch (System.Exception ex)
        {
            Debug.LogError("Error parsing JSON: " + ex.Message);
        }
    }

    private void OnWebSocketError(object sender, WebSocketSharp.ErrorEventArgs e)
    {
        Debug.LogError("WebSocket error: " + e.Message);
    }

    private void OnWebSocketClose(object sender, CloseEventArgs e)
    {
        Debug.Log("WebSocket closed.");
        if (outputText != null)
        {
            outputText.text = "Disconnected.";
        }
    }

    void OnDestroy()
    {
        // Close WebSocket connection when the object is destroyed
        if (webSocket != null && webSocket.IsAlive)
        {
            webSocket.Close();
        }
    }

    // Class to map JSON data from ESP32
    [System.Serializable]
    public class SensorData
    {
        public int accelX;
        public int accelY;
        public int accelZ;
        public int gyroX;
        public int gyroY;
        public int gyroZ;
    }
}
