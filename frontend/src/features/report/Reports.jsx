import { useState } from "react";
import IconButton from "../../components/ui/IconButton";
import Input from "../../components/ui/Input";
import TableLayout from "../dashboard/layout/TableLayout";
import agentCheckpoints from "../dashboard/data/agentCheckPoint";

const Reports = () => {
  const [searchQuery, setSearchQuery] = useState("");

  const normalizedQuery = searchQuery.trim().toLowerCase();

  const filteredCheckpoints = agentCheckpoints.filter((checkpoint) => {
    const searchableFields = [
      checkpoint.nomAppareil,
      checkpoint.nomAgent,
      checkpoint.idAgents,
      checkpoint.emplacement,
      checkpoint.idCheckpoint,
    ];

    return searchableFields.some((field) =>
      String(field ?? "")
        .toLowerCase()
        .includes(normalizedQuery),
    );
  });

  const formattedCount = String(filteredCheckpoints.length).padStart(2, "0");

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
          />
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
