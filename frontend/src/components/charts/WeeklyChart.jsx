import { useEffect, useState } from "react";
import {
  BarChart,
  Bar,
  XAxis,
  YAxis,
  CartesianGrid,
  ResponsiveContainer,
  Tooltip,
} from "recharts";
import weeklyPassagesService from "../../service/weeklyPassagesService";
import { connectWeeklyPassagesWebSocket } from "../../service/weeklyPassagesWebSocket";
import tokenService from "../../service/tokenService";
import useLanguage from "../../context/useLanguage";

export default function WeeklyChart() {
  const { t } = useLanguage();
  const [data, setData] = useState([]);
  const [error, setError] = useState("");

  useEffect(() => {
    let active = true;
    let receivedRealtimeData = false;

    const disconnect = connectWeeklyPassagesWebSocket({
      onWeeklyPassagesReceived: (passages) => {
        if (active && Array.isArray(passages)) {
          receivedRealtimeData = true;
          setData(passages);
          setError("");
        }
      },
      onAuthError: () => {
        tokenService.removeToken();
        window.location.replace("/signin");
      },
      onError: (webSocketError) => {
        console.error("Erreur WebSocket des passages hebdomadaires :", webSocketError);
      },
    });

    weeklyPassagesService
      .getAll()
      .then((passages) => {
        if (active && !receivedRealtimeData) {
          setData(passages);
          setError("");
        }
      })
      .catch((requestError) => {
        console.error("Impossible de charger les passages hebdomadaires :", requestError);
        if (active && !receivedRealtimeData) {
          setError("Impossible de charger les passages confirmés.");
        }
      });

    return () => {
      active = false;
      disconnect();
    };
  }, []);

  return (
    <div className="w-full h-64">
      {error && <p className="mb-2 text-sm text-red-600">{t(error)}</p>}
      <ResponsiveContainer width="100%" height="100%">
        <BarChart
          data={data}
          margin={{ top: 8, right: 4, left: -20, bottom: 0 }}
          barCategoryGap="35%"
        >
          <CartesianGrid vertical={false} stroke="#E5E7EB" strokeWidth={1} />

          <XAxis
            dataKey="day"
            axisLine={false}
            tickLine={false}
            tick={{
              fill: "#525252",
              fontSize: 12,
              fontWeight: 600,
            }}
            dy={8}
          />

          <YAxis
            domain={[0, "auto"]}
            axisLine={false}
            tickLine={false}
            tick={{
              fill: "#737373",
              fontSize: 12,
            }}
          />

          <Tooltip
            cursor={{ fill: "#F3E8FF", opacity: 0.4 }}
            formatter={(value) => [`${value}`, t("Valeur")]}
          />

          <Bar
            dataKey="value"
            fill="#A855E8"
            radius={[10, 10, 0, 0]}
            maxBarSize={36}
          />
        </BarChart>
      </ResponsiveContainer>
    </div>
  );
}
