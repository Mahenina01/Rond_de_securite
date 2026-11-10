import { useEffect, useState } from "react";
import Card from "../../../components/ui/Card";
import dashboardStatsService from "../../../service/dashboardStatsService";
import { connectDashboardStatsWebSocket } from "../../../service/dashboardStatsWebSocket";
import tokenService from "../../../service/tokenService";
import useLanguage from "../../../context/useLanguage";

const StatCard = () => {
  const { language, t } = useLanguage();
  const [stats, setStats] = useState(null);
  const [error, setError] = useState("");

  useEffect(() => {
    let active = true;
    let receivedRealtimeStats = false;

    const disconnect = connectDashboardStatsWebSocket({
      onStatsReceived: (nextStats) => {
        if (active) {
          receivedRealtimeStats = true;
          setStats(nextStats);
          setError("");
        }
      },
      onAuthError: () => {
        tokenService.removeToken();
        window.location.replace("/signin");
      },
      onError: (webSocketError) => {
        console.error("Erreur WebSocket des statistiques :", webSocketError);
      },
    });

    dashboardStatsService
      .get()
      .then((response) => {
        if (active && !receivedRealtimeStats) {
          setStats(response);
          setError("");
        }
      })
      .catch((requestError) => {
        console.error("Impossible de charger les statistiques :", requestError);
        if (active && !receivedRealtimeStats) {
          setError("Impossible de charger les statistiques du tableau de bord.");
        }
      });

    return () => {
      active = false;
      disconnect();
    };
  }, []);

  const value = (count) =>
    typeof count === "number"
      ? count.toLocaleString(language === "en" ? "en-US" : "fr-FR")
      : "—";

  const percentage = (percent) =>
    typeof percent === "number" ? `${percent.toFixed(1)} %` : "—";

  return (
    <>
      {error && <p className="text-sm text-red-600">{t(error)}</p>}
      <div className="grid grid-cols-1 gap-6 sm:grid-cols-2 lg:grid-cols-3">
        <Card
          variant="ronde_confirm"
          valueCount={value(stats?.confirmedRoundsToday)}
          evolution={stats?.roundsChangePercent}
        />
        <Card
          variant="device_actif_count"
          valueCount={value(stats?.activeDevices)}
          evolution={stats?.activeDevicesChangePercent}
        />
        <Card
          variant="succes_percent"
          valueCount={percentage(stats?.successRatePercent)}
          evolution={stats?.successRateChangePercent}
        />
      </div>
    </>
  );
};

export default StatCard;
