import { Client } from "@stomp/stompjs";
import SockJS from "sockjs-client";
import tokenService from "./tokenService";

const WS_URL = import.meta.env.VITE_WS_URL || "http://localhost:8022/ws-alerts";

export const connectAlertsWebSocket = ({
  onAlertEvent,
  onAuthError,
  onError,
}) => {
  const token = tokenService.getToken();

  if (!token) {
    onAuthError?.();
    return () => {};
  }

  const client = new Client({
    webSocketFactory: () => new SockJS(WS_URL),
    reconnectDelay: 5000,
    connectHeaders: {
      Authorization: `Bearer ${token}`,
    },
    onConnect: () => {
      client.subscribe("/topic/alerts", (message) => {
        try {
          onAlertEvent(JSON.parse(message.body));
        } catch (error) {
          onError?.(error);
        }
      });
    },
    onStompError: (frame) => {
      const message = frame.headers.message || frame.body || "";
      const authError =
        frame.headers["error-code"] === "UNAUTHORIZED" ||
        frame.headers["error-code"] === "FORBIDDEN" ||
        /unauthorized|forbidden|access denied|token/i.test(message);

      if (authError) {
        onAuthError?.();
        void client.deactivate();
        return;
      }

      onError?.(new Error(message));
    },
    onWebSocketError: onError,
  });

  void client.activate();
  return () => {
    void client.deactivate();
  };
};
