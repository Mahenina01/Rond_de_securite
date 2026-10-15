import { useState } from "react";
import Button from "../../components/ui/Button";
import Input from "../../components/ui/Input";
import DataTable from "../../components/ui/DataTable";
import Delete from "../../assets/icons/delete.svg?react";

export default function DeviceManagementPage() {
  // 1. État de la recherche
  const [searchQuery, setSearchQuery] = useState("");

  // 2. Variables de données initiales (Données dynamiques)
  const [devices, setDevices] = useState([
    { id: 1, name: "PN-02", agentId: "AG-02", key: "key-******" },
    { id: 2, name: "PN-03", agentId: "AG-03", key: "key-******" },
    { id: 3, name: "PN-04", agentId: "AG-04", key: "key-******" },
    { id: 4, name: "PN-05", agentId: "AG-05", key: "key-******" },
    { id: 5, name: "PN-06", agentId: "AG-06", key: "key-******" },
  ]);

  // Fonctions de gestion des actions
  const handleCopyKey = (keyVal) => {
    navigator.clipboard.writeText(keyVal);
    alert(`Clé copiée : ${keyVal}`);
  };

  const handleDelete = (id) => {
    setDevices(devices.filter((device) => device.id !== id));
  };

  const handleEdit = (device) => {
    alert(`Modification de l'appareil : ${device.name}`);
  };

  // 3. Filtrage dynamique selon la recherche
  const filteredDevices = devices.filter(
    (device) =>
      device.name.toLowerCase().includes(searchQuery.toLowerCase()) ||
      device.agentId.toLowerCase().includes(searchQuery.toLowerCase()),
  );

  // Formatage du compteur avec un zéro devant si < 10 (ex: 05)
  const formattedCount = String(filteredDevices.length).padStart(2, "0");

  // 4. Configuration des colonnes du tableau (avec rendu personnalisé pour les clés et les actions)
  const deviceColumns = [
    { header: "Nom appareil", accessor: "name" },
    { header: "ID Agents", accessor: "agentId" },
    {
      header: "clés",
      render: (row) => (
        <div className="flex items-center gap-2">
          <span>{row.key}</span>
          <button
            onClick={() => handleCopyKey(row.key)}
            className="text-gray-400 hover:text-gray-700 focus:outline-none"
            title="Copier la clé"
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
                d="M8 16H6a2 2 0 01-2-2V6a2 2 0 012-2h8a2 2 0 012 2v2m-6 12h8a2 2 0 002-2v-8a2 2 0 00-2-2h-8a2 2 0 00-2 2v8a2 2 0 002 2z"
              />
            </svg>
          </button>
        </div>
      ),
    },

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
    <div className="bg-white rounded-[16px] p-6 shadow-sm border border-gray-200 flex flex-col gap-6 w-full">
      {/* Ligne 1 : Titre et Bouton "Nouveau appareil" */}
      <div className="flex flex-col items-start justify-between gap-4 sm:flex-row sm:items-center">
        <h2 className="text-xl font-bold text-gray-900">
          Gestion des appareils
        </h2>
        <Button
          variant="primary"
          onClick={() => alert("Ouverture du formulaire d'ajout")}
          className="gap-2 text-sm"
        >
          <span className="text-lg leading-none">+</span> Nouveau appareil
        </Button>
      </div>

      {/* Ligne 2 : Champ de recherche */}
      <div className="w-full sm:w-[320px]">
        <Input
          variant="search"
          placeholder="Recherche..."
          value={searchQuery}
          onChange={(e) => setSearchQuery(e.target.value)}
        />
      </div>

      {/* Ligne 3 : Compteur de résultats dynamique */}
      <div className="text-sm font-semibold text-gray-800">
        Résultats ({formattedCount})
      </div>

      {/* Ligne 4 : Intégration du composant Tableau dynamique */}
      <DataTable columns={deviceColumns} data={filteredDevices} />
    </div>
  );
}
