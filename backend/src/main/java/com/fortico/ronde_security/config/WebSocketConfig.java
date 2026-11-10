package com.fortico.ronde_security.config;

import org.springframework.context.annotation.Configuration;
import org.springframework.messaging.simp.config.MessageBrokerRegistry;
import org.springframework.messaging.simp.config.ChannelRegistration;
import org.springframework.web.socket.config.annotation.EnableWebSocketMessageBroker;
import org.springframework.web.socket.config.annotation.StompEndpointRegistry;
import org.springframework.web.socket.config.annotation.WebSocketMessageBrokerConfigurer;

@Configuration
@EnableWebSocketMessageBroker
public class WebSocketConfig
    implements WebSocketMessageBrokerConfigurer {

  private final StompAuthChannelInterceptor authChannelInterceptor;

  public WebSocketConfig(
      StompAuthChannelInterceptor authChannelInterceptor) {
    this.authChannelInterceptor = authChannelInterceptor;
  }

  @Override
  public void configureMessageBroker(MessageBrokerRegistry config) {
    config.enableSimpleBroker("/topic");
    config.setApplicationDestinationPrefixes("/app");
  }

  @Override
  public void configureClientInboundChannel(ChannelRegistration registration) {
    registration.interceptors(authChannelInterceptor);
  }

  @Override
  public void registerStompEndpoints(StompEndpointRegistry registry) {
    registry.addEndpoint("/ws-alerts")
        .setAllowedOriginPatterns(
            "http://localhost:5173",
            "https://rond-de-securite.vercel.app",
            "https://rond-de-securite.onrender.com")
        .withSockJS();
  }
}