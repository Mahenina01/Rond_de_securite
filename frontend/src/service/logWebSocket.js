import { Client } from "@stomp/stompjs";
import SockJS from "sockjs-client";
import tokenService from "../service/tokenService";

const WS_URL = import.meta.env.VITE_WS_URL || "http://localhost:8022/ws-alerts";

export const connectLogWebSocket = ({ onLogReceived, onAuthError }) => {
  const token = tokenService.getToken();

  if (!token) {
    onAuthError?.();
    return () => {};
  }

  const client = new Client({
    webSocketFactory: () => new SockJS(WS_URL),

    // Réessayer après une coupure réseau
    reconnectDelay: 5000,

    // JWT transmis dans la trame STOMP CONNECT
    connectHeaders: {
      Authorization: `Bearer ${token}`,
    },

    onConnect: () => {
      console.log("WebSocket connecté");

      client.subscribe(
        "/topic/logs",
        (message) => {
          try {
            const log = JSON.parse(message.body);
            onLogReceived(log);
          } catch (error) {
            console.error("Impossible de lire le log :", error);
          }
        },
        {
          Authorization: `Bearer ${token}`,
        },
      );
    },

    onStompError: (frame) => {
      console.error("Erreur STOMP :", frame.headers.message || frame.body);

      // Le backend doit signaler explicitement les erreurs d'authentification
      const authError =
        frame.headers["error-code"] === "UNAUTHORIZED" ||
        frame.headers["error-code"] === "FORBIDDEN";

      if (authError) {
        onAuthError?.();
        void client.deactivate();
      }
    },

    onWebSocketError: (error) => {
      console.error("Erreur WebSocket :", error);
    },
  });

  void client.activate();

  return () => {
    void client.deactivate();
  };
};
