import { useState } from "react";
import Delete from "../../assets/icons/delete.svg?react";
import AddIcon from "../../assets/icons/add.svg?react";
import Input from "../../components/ui/Input";
import IconButton from "../../components/ui/IconButton";
import DataTable from "../../components/ui/DataTable";

const checkpointList = [
  {
    id: 1,
    idCheckpoint: "CP-65667R7",
    nameCheckpoint: "Salle 1",
    agent: "AG-024",
  },
  {
    id: 2,
    idCheckpoint: "CP-65667R7",
    nameCheckpoint: "Salle 2",
    agent: "AG-043",
  },
  {
    id: 3,
    idCheckpoint: "CP-65667R7",
    nameCheckpoint: "Salle 3",
    agent: "AG-054",
  },
  {
    id: 4,
    idCheckpoint: "CP-65667R7",
    nameCheckpoint: "Salle 4",
    agent: "AG-055",
  },
  {
    id: 5,
    idCheckpoint: "CP-65667R7",
    nameCheckpoint: "Salle 5",
    agent: "AG-54",
  },
  {
    id: 6,
    idCheckpoint: "CP-65667R7",
    nameCheckpoint: "Salle 6",
    agent: "AG-056",
  },
  {
    id: 7,
    idCheckpoint: "CP-65667R7",
    nameCheckpoint: "Salle 7",
    agent: "AG-057",
  },
  {
    id: 8,
    idCheckpoint: "CP-65667R7",
    nameCheckpoint: "Salle 8",
    agent: "AG-058",
  },
  {
    id: 9,
    idCheckpoint: "CP-65667R7",
    nameCheckpoint: "Salle 9",
    agent: "AG-059",
  },
  {
    id: 10,
    idCheckpoint: "CP-65667R7",
    nameCheckpoint: "Salle 10",
    agent: "AG-060",
  },
];

const Locations = () => {
  const [searchQuery, setSearchQuery] = useState("");

  const [checkpoint, setCheckpoint] = useState(checkpointList);

  const handleDelete = (id) => {
    setCheckpoint(checkpoint.filter((checkpoint) => checkpoint.id !== id));
  };

  const handleEdit = (checkpoint) => {
    alert(`Modification de point de contrôle : ${checkpoint.nameCheckpoint}`);
  };

  const filtereCheckpoint = checkpoint.filter(
    (checkpoint) =>
      checkpoint.nameCheckpoint
        .toLowerCase()
        .includes(searchQuery.toLowerCase()) ||
      checkpoint.idCheckpoint.toLowerCase().includes(searchQuery.toLowerCase()),
  );

  const formattedCount = String(filtereCheckpoint.length).padStart(2, "0");

  const checkpointColumns = [
    { header: "ID Checkpoint", accessor: "idCheckpoint" },
    { header: "Nom point de contrôle", accessor: "nameCheckpoint" },
    { header: "Reponsable", accessor: "agent" },
    {
      header: "Actions",
      align: "right",
      render: (row) => (
        <div className="flex items-center justify-end gap-3">
          <button
            onClick={() => handleEdit(row)}
            className="text-gray-500 transition-colors hover:text-blue-600"
            title="Modifier"
          >
            <svg
              xmlns="http://www.w3.org/2000/svg"
              className="w-4 h-4"
              fill="none"
              viewBox="0 0 24 24"
              stroke="currentColor"
            >
              <path
                strokeLinecap="round"
                strokeLinejoin="round"
                strokeWidth={2}
                d="M11 5H6a2 2 0 00-2 2v11a2 2 0 002 2h11a2 2 0 002-2v-5m-1.414-9.414a2 2 0 112.828 2.828L11.828 15H9v-2.828l8.586-8.586z"
              />
            </svg>
          </button>
          <button
            onClick={() => handleDelete(row.id)}
            className="text-red-500 transition-colors hover:text-red-700"
            title="Supprimer"
          >
            <Delete className="w-4 h-4 text-danger" />
          </button>
        </div>
      ),
    },
  ];
  return (
    <div className="flex flex-col gap-6 p-4 rounded-lg bg-surface">
      <h1 className="font-sans text-xl text-text-primary">
        Gestion des points de contrôle
      </h1>
      <div className="flex flex-row items-center justify-between w-full">
        <div className="w-full sm:w-[320px]">
          <Input
            variant="search"
            placeholder="Recherche..."
            value={searchQuery}
            onChange={(e) => setSearchQuery(e.target.value)}
          />
        </div>
        <IconButton isButton={true} iconSVG={AddIcon} text="Nouveau agents" />
      </div>
      <div className="text-sm font-semibold text-gray-800">
        Résultats ({formattedCount})
      </div>
      <DataTable columns={checkpointColumns} data={filtereCheckpoint} />
    </div>
  );
};

export default Locations;
