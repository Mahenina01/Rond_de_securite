import { useCallback, useEffect, useState } from "react";
import IconButton from "../../components/ui/IconButton";
import Input from "../../components/ui/Input";
import DataTable from "../../components/ui/DataTable";
import Modal from "../../components/ui/Modal";
import Button from "../../components/ui/Button";
import ExportIcon from "../../assets/icons/exporter.svg?react";
import RadioButton from "../../components/ui/RatioButton";
import { exportData } from "../../utils/exportData";
import logsService from "../../service/logsService";
import devicesService from "../../service/devciesService";
import toast from "react-hot-toast";
import useLanguage from "../../context/useLanguage";

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

const formatDate = (value, language) => {
  if (!value) return "—";

  const date = new Date(value);

  if (Number.isNaN(date.getTime())) return String(value);

  return date.toLocaleString(language === "en" ? "en-US" : "fr-FR", {
    year: "numeric",
    month: "2-digit",
    day: "2-digit",
    hour: "2-digit",
    minute: "2-digit",
    second: "2-digit",
  });
};

// Préparer les données pour le tableau et l'export.
const mapLog = (log, language) => ({
  ...log,
  dateHeure: formatDate(log.timestamp, language),
  idAgents: log.agentName ?? "—",
  nomAppareil: log.deviceCode ?? "—",
  emplacement:
    log.location !== null && log.location !== undefined
      ? `${log.location}`
      : "—",
  statut: log.status ?? "INCONNU",
  receivedAtFormatted: formatDate(log.receivedAt, language),
});

const EMPTY_FILTERS = {
  agentName: "",
  date: "",
  deviceId: "",
  locationName: "",
};

const toApiFilters = ({ agentName, date, deviceId, locationName }) => ({
  ...(agentName.trim() && { agent_name: agentName.trim() }),
  ...(date && { date }),
  ...(deviceId && { device_id: deviceId }),
  ...(locationName.trim() && { location_name: locationName.trim() }),
});

const Reports = () => {
  const { language, t } = useLanguage();
  const [logs, setLogs] = useState([]);
  const [loading, setLoading] = useState(true);
  const [devices, setDevices] = useState([]);
  const [filters, setFilters] = useState(EMPTY_FILTERS);
  const [appliedFilters, setAppliedFilters] = useState(EMPTY_FILTERS);
  const [searchQuery, setSearchQuery] = useState("");
  const [isFilterClicked, setIsFilterClicked] = useState(false);

  const [isModalOpen, setIsModalOpen] = useState(false);
  const [format, setFormat] = useState("csv");
  const [isExporting, setIsExporting] = useState(false);
  const reportColumns = [
    { header: "N°", dataKey: "id" },
    { header: t("Date et heure"), dataKey: "dateHeure" },
    { header: t("Agent"), dataKey: "idAgents" },
    { header: t("Appareil"), dataKey: "nomAppareil" },
    { header: t("Emplacement"), dataKey: "emplacement" },
    { header: t("Statut"), dataKey: "statut" },
    { header: t("UUID événement"), dataKey: "eventUuid" },
    { header: t("Date de réception"), dataKey: "receivedAtFormatted" },
  ];
  const reportTableColumns = reportColumns.map(({ header, dataKey }) => ({
    header,
    accessor: dataKey,
  }));

  const handleFilterClick = () => {
    setIsFilterClicked((prev) => !prev);
  };

  // Charger les logs depuis l'API.
  const loadLogs = useCallback(async (params = {}) => {
    try {
      const response = await logsService.getAll(params);
      const list = getResponseList(response);

      if (!list) {
        throw new Error(t("Le serveur n'a pas renvoyé une liste valide de logs."));
      }

      setLogs(list.map((log) => mapLog(log, language)));
    } catch (error) {
      console.error(t("Erreur de chargement des logs :"), error);

      toast.error(getErrorMessage(error, t("Impossible de charger les logs.")));
    } finally {
      setLoading(false);
    }
  }, [language, t]);

  useEffect(() => {
    let cancelled = false;

    const fetchInitialLogs = async () => {
      try {
        const response = await logsService.getAll();
        const list = getResponseList(response);

        if (!list) {
          throw new Error(
            t("Le serveur n'a pas renvoyé une liste valide de logs."),
          );
        }

        if (!cancelled) setLogs(list.map((log) => mapLog(log, language)));
      } catch (error) {
        if (!cancelled) {
          console.error(t("Erreur de chargement des logs :"), error);
          toast.error(
            getErrorMessage(error, t("Impossible de charger les logs.")),
          );
        }
      } finally {
        if (!cancelled) setLoading(false);
      }
    };

    fetchInitialLogs();

    return () => {
      cancelled = true;
    };
  }, [language, t]);

  useEffect(() => {
    const loadDevices = async () => {
      try {
        const response = await devicesService.getAll();
        const list = getResponseList(response);

        if (!list) {
          throw new Error(
            t("Le serveur n'a pas renvoyé une liste valide d'appareils."),
          );
        }

        setDevices(list);
      } catch (error) {
        console.error(t("Erreur de chargement des appareils :"), error);
        toast.error(
          getErrorMessage(
            error,
            t("Impossible de charger la liste des appareils."),
          ),
        );
      }
    };

    loadDevices();
  }, [t]);

  const handleApplyFilters = (event) => {
    event.preventDefault();
    setAppliedFilters(filters);
    setLoading(true);
    loadLogs(toApiFilters(filters));
  };

  const handleClearFilters = () => {
    setFilters(EMPTY_FILTERS);
    setAppliedFilters(EMPTY_FILTERS);
    setLoading(true);
    loadLogs();
  };

  // Recherche sur les informations disponibles.
  const normalizedQuery = searchQuery.trim().toLowerCase();

  const filteredLogs = logs.filter((log) =>
    [
      log.id,
      log.eventUuid,
      log.agentName,
      log.deviceCode,
      log.location,
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
  const hasActiveFilters = Object.values(appliedFilters).some((value) =>
    String(value).trim(),
  );

  // Exporter les logs filtrés.
  const handleConfirm = async () => {
    if (filteredLogs.length === 0) {
      toast.error(t("Aucune donnée à exporter."));
      return;
    }

    try {
      setIsExporting(true);

      await exportData({
        data: filteredLogs,
        columns: reportColumns,
        format,
        filename: "rapport-rondes",
        title: t("Rapport des rondes de sécurité"),
        description:
          t("Historique des événements enregistrés par les appareils de ronde."),
        logo: "/assets/logo.png",
      });

      setIsModalOpen(false);
      toast.success(t("Rapport exporté avec succès."));
    } catch (error) {
      console.error(t("Erreur lors de l'exportation :"), error);

      toast.error(
        error?.message || t("Une erreur est survenue lors de l'exportation."),
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
              {t("Rapports d'audit")}
            </h1>

            <p className="font-sans text-sm text-text-secondary">
              {t("Événements enregistrés par les dispositifs")}
            </p>
          </div>

          <IconButton
            className="w-auto"
            isButton={true}
            text={t("Générer rapport")}
            onClick={() => setIsModalOpen(true)}
          />
        </div>

        {/* Recherche locale dans les résultats chargés */}
        <div className="w-full sm:w-[320px]">
          <Input
            variant="searchFilter"
            placeholder={t("Rechercher un appareil, un agent...")}
            name="searchQuery"
            value={searchQuery}
            onFilterClick={() => handleFilterClick()}
            onChange={(valueOrEvent) => {
              const value =
                typeof valueOrEvent === "string"
                  ? valueOrEvent
                  : (valueOrEvent?.target?.value ?? "");

              setSearchQuery(value);
            }}
          />
        </div>

        <form
          onSubmit={handleApplyFilters}
          className={
            isFilterClicked
              ? "flex flex-col gap-4 p-4 border border-gray-200 rounded-lg"
              : "hidden"
          }
        >
          <h2 className="text-sm font-semibold text-text-primary">
            {t("Filtrer les rapports")}
          </h2>
          <div className="grid grid-cols-1 gap-4 sm:grid-cols-2 xl:grid-cols-4">
            <Input
              label={t("Nom de l'agent")}
              name="agentName"
              placeholder={t("Ex. Jean Dupont")}
              value={filters.agentName}
              onChange={(event) =>
                setFilters((current) => ({
                  ...current,
                  agentName: event.target.value,
                }))
              }
            />
            <div className="flex flex-col w-full gap-2">
              <label
                htmlFor="reportDate"
                className="text-sm font-medium text-gray-700"
              >
                {t("Date")}
              </label>
              <input
                id="reportDate"
                name="date"
                type="date"
                value={filters.date}
                onChange={(event) =>
                  setFilters((current) => ({
                    ...current,
                    date: event.target.value,
                  }))
                }
                className="w-full px-4 py-3 text-sm transition border rounded-lg outline-none border-text-muted bg-surface focus:border-primary focus:ring-2 focus:ring-blue-100"
              />
            </div>
            <div className="flex flex-col w-full gap-2">
              <label
                htmlFor="reportDeviceId"
                className="text-sm font-medium text-gray-700"
              >
                {t("Appareil")}
              </label>
              <select
                id="reportDeviceId"
                name="deviceId"
                value={filters.deviceId}
                onChange={(event) =>
                  setFilters((current) => ({
                    ...current,
                    deviceId: event.target.value,
                  }))
                }
                className="w-full px-4 py-3 text-sm transition border rounded-lg outline-none border-text-muted bg-surface focus:border-primary focus:ring-2 focus:ring-blue-100"
              >
                <option value="">{t("Tous les appareils")}</option>
                {devices.map((device) => (
                  <option key={device.id} value={device.id}>
                    {device.deviceCode ?? `${t("Appareil")} ${device.id}`}
                  </option>
                ))}
              </select>
            </div>
            <Input
              label={t("Emplacement")}
              name="locationName"
              placeholder={t("Ex. Salle B")}
              value={filters.locationName}
              onChange={(event) =>
                setFilters((current) => ({
                  ...current,
                  locationName: event.target.value,
                }))
              }
            />
          </div>
          <div className="flex flex-wrap gap-3">
            <Button type="submit" disabled={loading}>
              {loading ? t("Filtrage...") : t("Appliquer les filtres")}
            </Button>
            <Button
              type="button"
              variant="outline"
              onClick={handleClearFilters}
              disabled={loading}
            >
              {t("Effacer les filtres")}
            </Button>
          </div>
        </form>

        {/* Nombre de résultats */}
        <div className="text-sm font-semibold text-gray-800">
          {t("Résultats")} ({formattedCount})
        </div>

        {/* Tableau */}
        {loading ? (
          <p className="py-6 text-sm text-text-secondary">
            {t("Chargement des logs...")}
          </p>
        ) : filteredLogs.length > 0 ? (
          <DataTable columns={reportTableColumns} data={filteredLogs} />
        ) : (
          <p className="py-8 text-sm text-center text-text-secondary">
            {searchQuery.trim() || hasActiveFilters
              ? t("Aucun log ne correspond à vos critères.")
              : t("Aucun événement enregistré.")}
          </p>
        )}
      </div>

      {/* Modal d'exportation */}
      <Modal
        isOpen={isModalOpen}
        onClose={() => {
          if (!isExporting) setIsModalOpen(false);
        }}
        title={t("Exporter les logs")}
        IconTitle={ExportIcon}
        footer={
          <>
            <Button
              type="button"
              onClick={() => setIsModalOpen(false)}
              variant="outline"
              disabled={isExporting}
            >
              {t("Annuler")}
            </Button>

            <Button
              type="button"
              onClick={handleConfirm}
              variant="primary"
              disabled={isExporting || filteredLogs.length === 0}
            >
              {isExporting ? t("Exportation...") : t("Exporter")}
            </Button>
          </>
        }
      >
        <div className="flex flex-col gap-3">
          <p className="font-sans text-sm font-normal text-text-secondary">
            {t(
              "Téléchargez les journaux de ronde pour les consulter, les archiver ou les analyser.",
            )}
          </p>

          <h2 className="text-sm font-normal text-text-primary">
            {t("Choisissez le format d'exportation :")}
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
            {filteredLogs.length} {t("ligne(s) seront exportées.")}
          </p>
        </div>
      </Modal>
    </div>
  );
};

export default Reports;
