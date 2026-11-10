import { useEffect, useRef, useState } from "react";
import AlertIcon from "../../../assets/icons/alert 1.svg?react";
import AlertCard from "../../../components/ui/AlertCard";
import alertsService from "../../../service/alertsService";
import { connectAlertsWebSocket } from "../../../service/alertsWebSocket";
import tokenService from "../../../service/tokenService";
import useLanguage from "../../../context/useLanguage";

const AlertLayout = () => {
  const { t } = useLanguage();
  const [alertsData, setAlertsData] = useState([]);
  const [error, setError] = useState("");
  const alertsById = useRef(new Map());

  useEffect(() => {
    let active = true;
    const realtimeEvents = new Map();

    const publishAlerts = () => {
      setAlertsData(
        [...alertsById.current.values()].sort(
          (a, b) =>
            new Date(b.receivedAt ?? b.timestamp ?? 0).getTime() -
            new Date(a.receivedAt ?? a.timestamp ?? 0).getTime(),
        ),
      );
    };

    const disconnect = connectAlertsWebSocket({
      onAlertEvent: (event) => {
        if (!active || !event?.action || !event.alertId) return;

        realtimeEvents.set(event.alertId, event);
        if (event.action === "DELETE") {
          alertsById.current.delete(event.alertId);
        } else if (event.action === "UPSERT" && event.alert) {
          alertsById.current.set(event.alertId, event.alert);
        } else {
          return;
        }

        publishAlerts();
        setError("");
      },
      onAuthError: () => {
        tokenService.removeToken();
        window.location.replace("/signin");
      },
      onError: (webSocketError) => {
        console.error("Erreur WebSocket des alertes :", webSocketError);
      },
    });

    alertsService
      .getAll()
      .then((alerts) => {
        if (!active || !Array.isArray(alerts)) return;
        alertsById.current = new Map(alerts.map((alert) => [alert.id, alert]));
        realtimeEvents.forEach((event, alertId) => {
          if (event.action === "DELETE") {
            alertsById.current.delete(alertId);
          } else if (event.action === "UPSERT" && event.alert) {
            alertsById.current.set(alertId, event.alert);
          }
        });
        publishAlerts();
        setError("");
      })
      .catch((requestError) => {
        console.error("Impossible de charger les alertes :", requestError);
        if (active) {
          setError("          {t(error)}");
        }
      });

    return () => {
      active = false;
      disconnect();
    };
  }, []);

  return (
    <div className="flex h-[380px] min-h-0 flex-col gap-3 overflow-hidden rounded-lg bg-surface p-4 w-full">
      {/* Titre fixe */}
      <div className="flex flex-row items-center gap-2 shrink-0">
        <AlertIcon className="w-8 h-8 text-warning" />
        <h1 className="font-sans text-2xl text-text-primary">{t("Alertes")}</h1>
      </div>

      {/* Description fixe */}
      <p className="font-sans text-sm shrink-0 text-text-secondary">
        {t("Derniers événements détectés sur le terrain.")}
      </p>

      {/* Seule cette zone défile */}
      <div className="flex flex-col flex-1 min-h-0 gap-3 overflow-y-auto">
        {error && <p className="text-sm text-red-600">{error}</p>}
        {!error && alertsData.length === 0 && (
          <p className="text-sm text-text-secondary">
            {t("Aucune alerte enregistrée.")}
          </p>
        )}
        {alertsData.map((alert) => (
          <AlertCard key={alert.id} alertData={alert} />
        ))}
      </div>
    </div>
  );
};

export default AlertLayout;
