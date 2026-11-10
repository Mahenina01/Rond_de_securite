import { useCallback, useEffect, useState } from "react";

import Xicon from "../../../assets/icons/x.svg?react";
import CheckIcon from "../../../assets/icons/check-circle 1.svg?react";
import DataTable from "../../../components/ui/DataTable";

import api from "../../../api/axios.js";
import tokenService from "../../../service/tokenService";
import { connectLogWebSocket } from "../../../service/logWebSocket.js";
import useLanguage from "../../../context/useLanguage";

const MAX_LOGS = 10;

// Formater une date ISO en date et heure lisibles
const formatDateTime = (value, language) => {
  if (!value) return "—";

  const date = new Date(value);

  if (Number.isNaN(date.getTime())) return "—";

  return new Intl.DateTimeFormat(language === "en" ? "en-US" : "fr-FR", {
    day: "2-digit",
    month: "2-digit",
    year: "numeric",
    hour: "2-digit",
    minute: "2-digit",
    second: "2-digit",
  }).format(date);
};

const TableLayout = ({ data = [] }) => {
  const { language, t } = useLanguage();
  const [logs, setLogs] = useState(() => data.slice(0, MAX_LOGS));

  // Ajouter les nouveaux logs en tête sans doublons
  const handleNewLog = useCallback((newLog) => {
    setLogs((previousLogs) => {
      const newId = newLog.eventUuid ?? newLog.id;

      const filteredLogs = previousLogs.filter((log) => {
        const existingId = log.eventUuid ?? log.id;

        return existingId !== newId;
      });

      return [newLog, ...filteredLogs].slice(0, MAX_LOGS);
    });
  }, []);

  // Récupérer les logs existants via REST
  useEffect(() => {
    let cancelled = false;

    const fetchLogs = async () => {
      try {
        const response = await api.get("/logs");
        const result = response.data;

        const fetchedLogs = Array.isArray(result)
          ? result
          : (result.content ?? result.data ?? []);

        if (!cancelled) {
          setLogs((previousLogs) => {
            // Préserver les événements WebSocket reçus pendant la requête REST
            const mergedLogs = [...previousLogs, ...fetchedLogs];
            const uniqueLogs = Array.from(
              new Map(
                mergedLogs.map((log) => [log.eventUuid ?? log.id, log]),
              ).values(),
            );

            // Les plus récents d'abord, selon la date de réception
            uniqueLogs.sort(
              (a, b) =>
                new Date(b.receivedAt ?? b.timestamp ?? 0).getTime() -
                new Date(a.receivedAt ?? a.timestamp ?? 0).getTime(),
            );

            return uniqueLogs.slice(0, MAX_LOGS);
          });
        }
      } catch (error) {
        if (!cancelled) {
          console.error(
            "Erreur lors de la récupération des logs :",
            error.response?.data ?? error.message,
          );
        }
      }
    };

    void fetchLogs();

    return () => {
      cancelled = true;
    };
  }, []);

  // Connexion WebSocket
  useEffect(() => {
    const handleAuthError = () => {
      tokenService.removeToken();

      if (window.location.pathname !== "/signin") {
        window.location.replace("/signin");
      }
    };

    const disconnect = connectLogWebSocket({
      onLogReceived: handleNewLog,
      onAuthError: handleAuthError,
    });

    return disconnect;
  }, [handleNewLog]);

  const checkpointColumns = [
    {
      header: t("Date & heure"),
      accessor: "timestamp",
      render: (row) => <span>{formatDateTime(row.timestamp, language)}</span>,
    },
    {
      header: t("ID Agent"),
      accessor: "agentCode",
    },
    {
      header: t("Nom agent"),
      accessor: "agentName",
      render: (row) => row.agentName || "—",
    },
    {
      header: t("Nom appareil"),
      accessor: "deviceCode",
    },
    {
      header: t("Emplacement"),
      accessor: "location",
      render: (row) => row.location ?? "—",
    },
    {
      header: t("Statut"),
      accessor: "status",
      render: (row) => {
        const isSuccess = row.status === "SUCCESS";

        return (
          <div className="flex items-center gap-2">
            {isSuccess ? (
              <CheckIcon className="w-4 h-4 text-success" />
            ) : (
              <Xicon className="w-4 h-4 text-danger" />
            )}

            <span>{isSuccess ? t("Validé") : t("Non validé")}</span>
          </div>
        );
      },
    },
  ];

  return <DataTable columns={checkpointColumns} data={logs} />;
};

export default TableLayout;
