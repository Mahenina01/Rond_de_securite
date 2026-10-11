package com.fortico.ronde_security.config;

import org.eclipse.paho.client.mqttv3.MqttConnectOptions;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.integration.annotation.ServiceActivator;
import org.springframework.integration.channel.DirectChannel;
import org.springframework.integration.core.MessageProducer;
import org.springframework.integration.mqtt.core.DefaultMqttPahoClientFactory;
import org.springframework.integration.mqtt.core.MqttPahoClientFactory;
import org.springframework.integration.mqtt.inbound.MqttPahoMessageDrivenChannelAdapter;
import org.springframework.integration.mqtt.outbound.MqttPahoMessageHandler;
import org.springframework.messaging.MessageChannel;
import org.springframework.messaging.MessageHandler;

@Configuration
public class MqttConfig {

  @Value("${mqtt.broker-url}")
  private String brokerUrl;

  @Value("${mqtt.client-id}")
  private String clientId;

  @Value("${mqtt.username:}")
  private String username;

  @Value("${mqtt.password:}")
  private String password;

  @Value("${mqtt.data-topic}")
  private String dataTopic;

  /**
   * Configure la connexion au broker MQTT
   */
  @Bean
  public MqttPahoClientFactory mqttClientFactory() {
    MqttConnectOptions options = new MqttConnectOptions();

    options.setServerURIs(new String[] { brokerUrl });
    options.setAutomaticReconnect(true);
    options.setCleanSession(true);
    options.setConnectionTimeout(10);
    options.setKeepAliveInterval(30);

    if (username != null && !username.isBlank()) {
      options.setUserName(username);

      if (password != null && !password.isBlank()) {
        options.setPassword(password.toCharArray());
      }
    }

    DefaultMqttPahoClientFactory factory = new DefaultMqttPahoClientFactory();

    factory.setConnectionOptions(options);

    return factory;
  }

  /**
   * Canal de réception des messages MQTT
   */
  @Bean
  public MessageChannel mqttInputChannel() {
    return new DirectChannel();
  }

  /**
   * Abonnement aux messages envoyés par tous les boîtiers
   */
  @Bean
  public MessageProducer mqttInbound(
      MqttPahoClientFactory mqttClientFactory) {

    MqttPahoMessageDrivenChannelAdapter adapter = new MqttPahoMessageDrivenChannelAdapter(
        brokerUrl,
        clientId + "-in",
        mqttClientFactory,
        dataTopic);

    adapter.setCompletionTimeout(5000);
    adapter.setQos(1);
    adapter.setOutputChannel(mqttInputChannel());

    return adapter;
  }

  /**
   * Canal de publication vers les boîtiers.
   */
  @Bean
  public MessageChannel mqttOutboundChannel() {
    return new DirectChannel();
  }

  /**
   * Gestionnaire des messages sortants.
   */
  @Bean
  @ServiceActivator(inputChannel = "mqttOutboundChannel")
  public MessageHandler mqttOutbound(
      MqttPahoClientFactory mqttClientFactory) {

    MqttPahoMessageHandler handler = new MqttPahoMessageHandler(
        brokerUrl,
        clientId + "-out",
        mqttClientFactory);

    handler.setAsync(false);
    handler.setDefaultQos(1);
    handler.setDefaultRetained(false);

    return handler;
  }
}
