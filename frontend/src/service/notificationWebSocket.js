import { Client } from "@stomp/stompjs";
import SockJS from "sockjs-client";
import tokenService from "./tokenService";

const WS_URL = import.meta.env.VITE_WS_URL || "http://localhost:8022/ws-alerts";

export const connectNotificationWebSocket = ({
  onNotificationReceived,
  onError,
}) => {
  const token = tokenService.getToken();
  if (!token) return () => {};

  const client = new Client({
    webSocketFactory: () => new SockJS(WS_URL),
    reconnectDelay: 5000,
    connectHeaders: {
      Authorization: `Bearer ${token}`,
    },
    onConnect: () => {
      client.subscribe("/topic/notifications", (message) => {
        try {
          onNotificationReceived(JSON.parse(message.body));
        } catch (error) {
          onError?.(error);
        }
      });
    },
    onStompError: (frame) => {
      onError?.(new Error(frame.headers.message || frame.body || "Erreur STOMP"));
    },
    onWebSocketError: onError,
  });

  void client.activate();
  return () => {
    void client.deactivate();
  };
};
