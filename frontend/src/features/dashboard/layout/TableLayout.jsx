import Xicon from "../../../assets/icons/x.svg?react";
import CheckIcon from "../../../assets/icons/check-circle 1.svg?react";
import DataTable from "../../../components/ui/DataTable";

const TableLayout = ({ data }) => {
  const checkpointColumns = [
    { header: "Date & heure", accessor: "dateHeure" },
    { header: "ID Agent", accessor: "idAgents" },
    { header: "Nom agent", accessor: "nomAgent" },
    { header: "Nom appareil", accessor: "nomAppareil" },
    { header: "Point de contrôle", accessor: "emplacement" },
    { header: "ID checkpoint", accessor: "idCheckpoint" },
    {
      header: "Statut",
      accessor: "statut",
      render: (row) => (
        <div className="flex items-center gap-2">
          {row.statut ? (
            <CheckIcon className="w-4 h-4 text-success" />
          ) : (
            <Xicon className="w-4 h-4 text-danger" />
          )}
          <span>{row.statut ? "Validé" : "Non validé"}</span>
        </div>
      ),
    },
  ];

  return <DataTable columns={checkpointColumns} data={data} />;
};

export default TableLayout;
