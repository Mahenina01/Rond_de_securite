import { useCallback, useEffect, useState } from "react";
import IconButton from "../../components/ui/IconButton";
import Input from "../../components/ui/Input";
import TableLayout from "../dashboard/layout/TableLayout";
import Modal from "../../components/ui/Modal";
import Button from "../../components/ui/Button";
import ExportIcon from "../../assets/icons/exporter.svg?react";
import RadioButton from "../../components/ui/RatioButton";
import { exportData } from "../../utils/exportData";
import logsService from "../../service/logsService";
import toast from "react-hot-toast";

const REPORT_COLUMNS = [
  { header: "N°", dataKey: "id" },
  { header: "Date et heure", dataKey: "dateHeure" },
  { header: "Agent", dataKey: "idAgents" },
  { header: "Appareil", dataKey: "nomAppareil" },
  { header: "Emplacement", dataKey: "emplacement" },
  { header: "Statut", dataKey: "statut" },
  { header: "UUID événement", dataKey: "eventUuid" },
  { header: "Date de réception", dataKey: "receivedAtFormatted" },
];

const getErrorMessage = (error, fallback) =>
  error?.response?.data?.message ||
  error?.response?.data?.detail ||
  error?.message ||
  fallback;

const getResponseList = (response) => {
  const data = response?.data ?? response;

  if (Array.isArray(data)) return data;
  if (Array.isArray(data?.content)) return data.content;
  if (Array.isArray(data?.data)) return data.data;
  if (Array.isArray(data?.data?.content)) return data.data.content;

  return null;
};

const formatDate = (value) => {
  if (!value) return "—";

  const date = new Date(value);

  if (Number.isNaN(date.getTime())) return String(value);

  return date.toLocaleString("fr-FR", {
    year: "numeric",
    month: "2-digit",
    day: "2-digit",
    hour: "2-digit",
    minute: "2-digit",
    second: "2-digit",
  });
};

// Préparer les données pour le tableau et l'export.
const mapLog = (log) => ({
  ...log,
  dateHeure: formatDate(log.timestamp),
  idAgents: log.agentCode ?? "—",
  nomAppareil: log.deviceCode ?? "—",
  emplacement:
    log.locationId !== null && log.locationId !== undefined
      ? `Point de contrôle #${log.locationId}`
      : "—",
  statut: log.status ?? "INCONNU",
  receivedAtFormatted: formatDate(log.receivedAt),
});

const Reports = () => {
  const [logs, setLogs] = useState([]);
  const [loading, setLoading] = useState(true);
  const [searchQuery, setSearchQuery] = useState("");

  const [isModalOpen, setIsModalOpen] = useState(false);
  const [format, setFormat] = useState("csv");
  const [isExporting, setIsExporting] = useState(false);

  // Charger les logs depuis l'API.
  const loadLogs = useCallback(async () => {
    setLoading(true);

    try {
      const response = await logsService.getAll();
      const list = getResponseList(response);

      if (!list) {
        throw new Error("Le serveur n'a pas renvoyé une liste valide de logs.");
      }

      setLogs(list.map(mapLog));
    } catch (error) {
      console.error("Erreur de chargement des logs :", error);

      toast.error(getErrorMessage(error, "Impossible de charger les logs."));
    } finally {
      setLoading(false);
    }
  }, []);

  useEffect(() => {
    loadLogs();
  }, [loadLogs]);

  // Recherche sur les informations disponibles.
  const normalizedQuery = searchQuery.trim().toLowerCase();

  const filteredLogs = logs.filter((log) =>
    [
      log.id,
      log.eventUuid,
      log.agentCode,
      log.deviceCode,
      log.locationId,
      log.status,
      log.dateHeure,
      log.receivedAtFormatted,
    ].some((field) =>
      String(field ?? "")
        .toLowerCase()
        .includes(normalizedQuery),
    ),
  );

  const formattedCount = String(filteredLogs.length).padStart(2, "0");

  // Exporter les logs filtrés.
  const handleConfirm = async () => {
    if (filteredLogs.length === 0) {
      toast.error("Aucune donnée à exporter.");
      return;
    }

    try {
      setIsExporting(true);

      await exportData({
        data: filteredLogs,
        columns: REPORT_COLUMNS,
        format,
        filename: "rapport-rondes",
        title: "Rapport des rondes de sécurité",
        description:
          "Historique des événements enregistrés par les appareils de ronde.",
        logo: "/assets/logo.png",
      });

      setIsModalOpen(false);
      toast.success("Rapport exporté avec succès.");
    } catch (error) {
      console.error("Erreur lors de l'exportation :", error);

      toast.error(
        error?.message || "Une erreur est survenue lors de l'exportation.",
      );
    } finally {
      setIsExporting(false);
    }
  };

  return (
    <div className="flex flex-col gap-6">
      <div className="flex flex-col flex-1 gap-4 p-4 rounded-lg bg-surface">
        {/* En-tête */}
        <div className="flex flex-col items-start justify-between gap-4 sm:flex-row sm:items-center">
          <div className="flex flex-col flex-1 gap-2">
            <h1 className="font-sans text-xl text-text-primary">
              Rapports d'audit
            </h1>

            <p className="font-sans text-sm text-text-secondary">
              Événements enregistrés par les dispositifs
            </p>
          </div>

          <IconButton
            className="w-auto"
            isButton={true}
            text="Générer rapport"
            onClick={() => setIsModalOpen(true)}
          />
        </div>

        {/* Recherche */}
        <div className="w-full sm:w-[320px]">
          <Input
            variant="search"
            placeholder="Rechercher un appareil, un agent..."
            name="searchQuery"
            value={searchQuery}
            onChange={(valueOrEvent) => {
              const value =
                typeof valueOrEvent === "string"
                  ? valueOrEvent
                  : (valueOrEvent?.target?.value ?? "");

              setSearchQuery(value);
            }}
          />
        </div>

        {/* Nombre de résultats */}
        <div className="text-sm font-semibold text-gray-800">
          Résultats ({formattedCount})
        </div>

        {/* Tableau */}
        {loading ? (
          <p className="py-6 text-sm text-text-secondary">
            Chargement des logs...
          </p>
        ) : filteredLogs.length > 0 ? (
          <TableLayout data={filteredLogs} />
        ) : (
          <p className="py-8 text-sm text-center text-text-secondary">
            {searchQuery.trim()
              ? "Aucun log ne correspond à votre recherche."
              : "Aucun événement enregistré."}
          </p>
        )}
      </div>

      {/* Modal d'exportation */}
      <Modal
        isOpen={isModalOpen}
        onClose={() => {
          if (!isExporting) setIsModalOpen(false);
        }}
        title="Exporter les logs"
        IconTitle={ExportIcon}
        footer={
          <>
            <Button
              type="button"
              onClick={() => setIsModalOpen(false)}
              variant="outline"
              disabled={isExporting}
            >
              Annuler
            </Button>

            <Button
              type="button"
              onClick={handleConfirm}
              variant="primary"
              disabled={isExporting || filteredLogs.length === 0}
            >
              {isExporting ? "Exportation..." : "Exporter"}
            </Button>
          </>
        }
      >
        <div className="flex flex-col gap-3">
          <p className="font-sans text-sm font-normal text-text-secondary">
            Téléchargez les journaux de ronde pour les consulter, les archiver
            ou les analyser.
          </p>

          <h2 className="text-sm font-normal text-text-primary">
            Choisissez le format d'exportation :
          </h2>

          <RadioButton
            label="CSV"
            selected={format}
            value="csv"
            onChange={setFormat}
          />

          <RadioButton
            label="Excel (.xlsx)"
            selected={format}
            value="xlsx"
            onChange={setFormat}
          />

          <RadioButton
            label="PDF"
            selected={format}
            value="pdf"
            onChange={setFormat}
          />

          <p className="text-xs text-text-secondary">
            {filteredLogs.length} ligne(s) seront exportées.
          </p>
        </div>
      </Modal>
    </div>
  );
};

export default Reports;
