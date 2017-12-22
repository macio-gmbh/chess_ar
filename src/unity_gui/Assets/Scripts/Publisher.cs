using CymaticLabs.Unity3D.Amqp;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Publisher : MonoBehaviour
{

    [Tooltip("The name of the exchange to subscribe to.")]
    public string exchangeName;

    [Tooltip("The exchange type for the exchange being subscribed to. It is important to get this value correct as the RabbitMQ client will close a connection if you pass the wrong type for an already declared exchange.")]
    public AmqpExchangeTypes exchangeType = AmqpExchangeTypes.Topic;

    [Tooltip("The optional routing key to use when subscribing to the exchange. This mostly applies to 'topic' exchanges.")]
    public string routingKey;

    public GuiManager UiManager;

    // Use this for initialization
    void Start()
    {
        var subscription = new UnityAmqpExchangeSubscription(exchangeName, exchangeType, routingKey, null, AmqpClient.Instance.UnityEventDebugExhangeMessageHandler);

        // Subscribe on the client
        AmqpClient.Subscribe(subscription);
    }

    public void Publish(string message)
    {
        AmqpClient.Publish(exchangeName, routingKey, message);
        if (message == "quit")
        {
            Application.Quit();
        }
    }
}
