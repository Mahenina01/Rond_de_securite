import { useState } from "react";
import IconButton from "../../components/ui/IconButton";
import Input from "../../components/ui/Input";
import TableLayout from "../dashboard/layout/TableLayout";
import agentCheckpoints from "../dashboard/data/agentCheckPoint";
import Modal from "../../components/ui/Modal";
import Button from "../../components/ui/Button";
import ExportIcon from "../../assets/icons/exporter.svg?react";
import RadioButton from "../../components/ui/RatioButton";
import { exportData } from "../../utils/exportData";

const REPORT_COLUMNS = [
  { header: "N°", dataKey: "id" },
  { header: "Date et heure", dataKey: "dateHeure" },
  { header: "Agent", dataKey: "idAgents" },
  { header: "Appareil", dataKey: "nomAppareil" },
  { header: "Emplacement", dataKey: "emplacement" },
  { header: "Checkpoint", dataKey: "idCheckpoint" },
  {
    header: "Statut",
    dataKey: "statut",
    formatter: (value) => (value ? "Présent" : "Absent"),
  },
];

const Reports = () => {
  const [isModalOpen, setIsModalOpen] = useState(false);
  const [searchQuery, setSearchQuery] = useState("");
  const [format, setFormat] = useState("csv");
  const [isExporting, setIsExporting] = useState(false);

  const normalizedQuery = searchQuery.trim().toLowerCase();

  const filteredCheckpoints = agentCheckpoints.filter((checkpoint) => {
    const searchableFields = [
      checkpoint.nomAppareil,
      checkpoint.nomAgent,
      checkpoint.idAgents,
      checkpoint.emplacement,
      checkpoint.idCheckpoint,
      checkpoint.dateHeure,
      checkpoint.statut ? "présent" : "absent",
    ];

    return searchableFields.some((field) =>
      String(field ?? "")
        .toLowerCase()
        .includes(normalizedQuery),
    );
  });

  const formattedCount = String(filteredCheckpoints.length).padStart(2, "0");

  const handleConfirm = async () => {
    if (filteredCheckpoints.length === 0) {
      alert("Aucune donnée à exporter.");
      return;
    }

    try {
      setIsExporting(true);

      await exportData({
        data: filteredCheckpoints,
        columns: REPORT_COLUMNS,
        format,
        filename: "rapport-rondes",
        title: "Rapport des rondes de sécurité",
        description:
          "Historique des passages des agents aux points de contrôle.",
        logo: "/assets/logo.png",
      });

      setIsModalOpen(false);
    } catch (error) {
      console.error("Erreur lors de l'exportation :", error);
      alert(error.message || "Une erreur est survenue lors de l'exportation.");
    } finally {
      setIsExporting(false);
    }
  };

  return (
    <div className="flex flex-col gap-6">
      <div className="flex flex-col flex-1 gap-4 p-4 rounded-lg bg-surface">
        <div className="flex flex-row items-center">
          <div className="flex flex-col flex-1 gap-4">
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
                  children="Annuler"
                  variant="outline"
                  disabled={isExporting}
                />

                <Button
                  type="button"
                  onClick={handleConfirm}
                  variant="primary"
                  disabled={isExporting || filteredCheckpoints.length === 0}
                >
                  {isExporting ? "Exportation..." : "Exporter"}
                </Button>
              </>
            }
          >
            <div className="flex flex-col gap-3">
              <p className="font-sans text-sm font-normal text-text-secondary">
                Téléchargez les journaux de ronde pour les consulter, les
                archiver ou les analyser.
              </p>

              <h2 className="text-sm font-normal text-text-primary">
                Choisissez le format d’exportation :
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
                {filteredCheckpoints.length} ligne(s) seront exportées.
              </p>
            </div>
          </Modal>
        </div>

        <div className="w-full sm:w-[320px]">
          <Input
            variant="search"
            placeholder="Rechercher un appareil, un agent..."
            value={searchQuery}
            onChange={(value) =>
              setSearchQuery(
                typeof value === "string" ? value : value.target.value,
              )
            }
          />
        </div>

        <div className="text-sm font-semibold text-gray-800">
          Résultats ({formattedCount})
        </div>

        <TableLayout data={filteredCheckpoints} />
      </div>
    </div>
  );
};

export default Reports;
