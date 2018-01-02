using CymaticLabs.Unity3D.Amqp;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Receiver : MonoBehaviour
{
    [Tooltip("The name of the exchange to subscribe to.")]
    public string ExchangeName;

    [Tooltip("The exchange type for the exchange being subscribed to. It is important to get this value correct as the RabbitMQ client will close a connection if you pass the wrong type for an already declared exchange.")]
    public AmqpExchangeTypes ExchangeType = AmqpExchangeTypes.Topic;

    [Tooltip("The optional routing key to use when subscribing to the exchange. This mostly applies to 'topic' exchanges.")]
    public string RoutingKey;

    [Tooltip("GameObject with script to generate the path for best move.")]
    public PathGenerator PathController;

    [Tooltip("Animator to toggle screen info.")]
    public Animator GuiAnimator;

    public Animator ErrorAnimator;

    public GuiManager UiManager;

    private string fenString = "";

    private bool fenReceived = false;

    private AmqpExchangeReceivedMessage lastReceivedMessage;


    // Use this for initialization
    void Start()
    {

        var subscription = new AmqpExchangeSubscription(ExchangeName, ExchangeType, RoutingKey, HandleExchangeMessageReceived);

        /*
         * Add the subscription to the client. If you are using multiple AmqpClient instances then
         * using the static methods won't work. In that case add a inspector property of type 'AmqpClient'
         * and assigned a reference to the connection you want to work with and call the 'SubscribeToExchange()'
         * non-static method instead.
         */
        AmqpClient.Subscribe(subscription);
    }
    
    /**
     * Handles messages receieved from this object's subscription based on the exchange name,
     * exchange type, and routing key used. You could also write an anonymous delegate in line
     * when creating the subscription like: (received) => { Debug.Log(received.Message.Body.Length); }
     */
    void HandleExchangeMessageReceived(AmqpExchangeReceivedMessage received)
    {
        string receivedString = System.Text.Encoding.UTF8.GetString(received.Message.Body);

        if (receivedString != fenString)
        {
            if (fenReceived && !GuiAnimator.GetBool("IsDisplayed"))
            {
                if (!ErrorAnimator.GetBool("errorIsShown"))
                {
                    GuiAnimator.SetTrigger("update");
                }
                StartCoroutine(Wait(received));
                lastReceivedMessage = received;
                Debug.Log("update");
            }
            else if (fenReceived && GuiAnimator.GetBool("IsDisplayed"))
            {
                lastReceivedMessage = received;
                Debug.Log("stored");
            }
            else
            {
                lastReceivedMessage = received;
                StartCoroutine(Wait(received));
                Debug.Log("first");
            }
        }
    }

    IEnumerator Wait(AmqpExchangeReceivedMessage received)
    {
        yield return new WaitForSeconds(0.25F);
        // First convert the message's body, which is a byte array, into a string
        // example fen string: rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 g1f3
        string receivedString = System.Text.Encoding.UTF8.GetString(received.Message.Body);
        if (receivedString.Substring(0, 5) == "ERROR")
        {
            PathController.receiveError(receivedString);
            ErrorAnimator.SetBool("errorIsShown", true);
        }
        else if (receivedString.Substring(0, 4) == "CHECK")
        {
            PathController.receiveError(receivedString);
            ErrorAnimator.SetBool("errorIsShown", true);
        }
        else if (receivedString.Split(' ')[0] == "CHECKMATED")
        {
            GuiAnimator.SetTrigger("show");
            UiManager.enableDarkOverlay();
            UiManager.enableCheckmatedPanel();
            UiManager.disableWaitingPanel();
            UiManager.setCurrentPlayer(receivedString.Split(' ')[1]);
        }
        else
        {
            GuiAnimator.SetTrigger("show");
            fenString = receivedString.Replace(System.Environment.NewLine, "");
            PathController.receiveFen(fenString);
            fenReceived = true;
            ErrorAnimator.SetBool("errorIsShown", false);
            UiManager.disableDarkOverlay();
            UiManager.disableCheckmatedPanel();
            UiManager.disableWaitingPanel();
        }
    }

    public void update()
    {
        StartCoroutine(Wait(lastReceivedMessage));
    }
}
