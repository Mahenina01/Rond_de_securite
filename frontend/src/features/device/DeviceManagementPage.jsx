import { useState } from "react";
import Button from "../../components/ui/Button";
import Input from "../../components/ui/Input";
import DataTable from "../../components/ui/DataTable";
import Delete from "../../assets/icons/delete.svg?react";
import Modal from "../../components/ui/Modal";
import DeviceIcon from "../../assets/icons/device-out.svg?react";

const initialDevices = [
  { id: 1, name: "PN-02", agentId: "AG-02", key: "key-******" },
  { id: 2, name: "PN-03", agentId: "AG-03", key: "key-******" },
  { id: 3, name: "PN-04", agentId: "AG-04", key: "key-******" },
  { id: 4, name: "PN-05", agentId: "AG-05", key: "key-******" },
  { id: 5, name: "PN-06", agentId: "AG-06", key: "key-******" },
];

const emptyForm = {
  name: "",
  agentId: "",
};

export default function DeviceManagementPage() {
  const [isModalOpen, setIsModalOpen] = useState(false);
  const [searchQuery, setSearchQuery] = useState("");
  const [devices, setDevices] = useState(initialDevices);
  const [formData, setFormData] = useState(emptyForm);
  const [formError, setFormError] = useState("");

  // Gestion des champs du formulaire
  const handleChangeFor = (name) => (valueOrEvent) => {
    const value =
      typeof valueOrEvent === "string"
        ? valueOrEvent
        : (valueOrEvent?.target?.value ?? "");

    setFormData((previous) => ({
      ...previous,
      [name]: value,
    }));

    setFormError("");
  };

  // Réinitialisation et fermeture du formulaire
  const handleCloseModal = () => {
    setIsModalOpen(false);
    setFormData(emptyForm);
    setFormError("");
  };

  // Ajout d'un nouvel appareil
  const handleConfirm = () => {
    const name = formData.name.trim().toUpperCase();
    const agentId = formData.agentId.trim().toUpperCase();

    if (!name || !agentId) {
      setFormError("Veuillez remplir tous les champs obligatoires.");
      return;
    }

    const deviceExists = devices.some(
      (device) => device.name.toLowerCase() === name.toLowerCase(),
    );

    if (deviceExists) {
      setFormError("Un appareil avec cet identifiant existe déjà.");
      return;
    }

    const newDevice = {
      id:
        devices.length > 0
          ? Math.max(...devices.map((device) => device.id)) + 1
          : 1,
      name,
      agentId,
      key: "key-******",
    };

    setDevices((previous) => [...previous, newDevice]);

    handleCloseModal();
  };

  // Copie de la clé
  const handleCopyKey = async (keyVal) => {
    try {
      await navigator.clipboard.writeText(keyVal);
      alert(`Clé copiée : ${keyVal}`);
    } catch {
      alert("Impossible de copier la clé.");
    }
  };

  // Suppression d'un appareil
  const handleDelete = (id) => {
    setDevices((previous) => previous.filter((device) => device.id !== id));
  };

  // Modification d'un appareil
  const handleEdit = (device) => {
    alert(`Modification de l'appareil : ${device.name}`);
  };

  // Recherche dynamique
  const filteredDevices = devices.filter((device) => {
    const query = searchQuery.trim().toLowerCase();

    return (
      device.name.toLowerCase().includes(query) ||
      device.agentId.toLowerCase().includes(query)
    );
  });

  const formattedCount = String(filteredDevices.length).padStart(2, "0");

  const isFormValid =
    formData.name.trim() !== "" && formData.agentId.trim() !== "";

  // Configuration des colonnes
  const deviceColumns = [
    {
      header: "Nom appareil",
      accessor: "name",
    },
    {
      header: "ID Agents",
      accessor: "agentId",
    },
    {
      header: "Clés",
      render: (row) => (
        <div className="flex items-center gap-2">
          <span>{row.key}</span>

          <button
            type="button"
            onClick={() => handleCopyKey(row.key)}
            className="text-gray-400 hover:text-gray-700 focus:outline-none"
            title="Copier la clé"
            aria-label={`Copier la clé de ${row.name}`}
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
            type="button"
            onClick={() => handleEdit(row)}
            className="text-gray-500 transition-colors hover:text-blue-600"
            title="Modifier"
            aria-label={`Modifier ${row.name}`}
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
            type="button"
            onClick={() => handleDelete(row.id)}
            className="text-red-500 transition-colors hover:text-red-700"
            title="Supprimer"
            aria-label={`Supprimer ${row.name}`}
          >
            <Delete className="w-4 h-4 text-danger" />
          </button>
        </div>
      ),
    },
  ];

  return (
    <div className="flex w-full flex-col gap-6 rounded-[16px] border border-gray-200 bg-white p-6 shadow-sm">
      {/* Titre et bouton d'ajout */}
      <div className="flex flex-col items-start justify-between gap-4 sm:flex-row sm:items-center">
        <h2 className="text-xl font-bold text-gray-900">
          Gestion des appareils
        </h2>

        <Button
          variant="primary"
          onClick={() => {
            setFormData(emptyForm);
            setFormError("");
            setIsModalOpen(true);
          }}
          className="gap-2 text-sm"
        >
          <span className="text-lg leading-none">+</span>
          Nouveau appareil
        </Button>
      </div>

      {/* Modal d'ajout */}
      <Modal
        isOpen={isModalOpen}
        onClose={handleCloseModal}
        title="Ajout d'un appareil"
        IconTitle={DeviceIcon}
        footer={
          <>
            <Button type="button" onClick={handleCloseModal} variant="outline">
              Annuler
            </Button>

            <Button
              type="button"
              onClick={handleConfirm}
              variant="primary"
              disabled={!isFormValid}
            >
              Ajouter
            </Button>
          </>
        }
      >
        <div className="flex flex-col gap-3">
          <p className="font-sans text-sm font-normal text-text-secondary">
            Enregistrez un appareil et associez-le à un agent pour assurer le
            suivi de ses rondes de sécurité.
          </p>

          <Input
            variant="default"
            label="ID de l'appareil"
            name="name"
            placeholder="Ex. : PN-001"
            value={formData.name}
            onChange={handleChangeFor("name")}
            required
          />

          <Input
            variant="default"
            label="Agent responsable"
            name="agentId"
            placeholder="Ex. : AG-001"
            value={formData.agentId}
            onChange={handleChangeFor("agentId")}
            required
          />

          {formError && (
            <p className="text-sm text-danger" role="alert">
              {formError}
            </p>
          )}
        </div>
      </Modal>

      {/* Recherche */}
      <div className="w-full sm:w-[320px]">
        <Input
          variant="search"
          placeholder="Recherche..."
          value={searchQuery}
          onChange={(valueOrEvent) =>
            setSearchQuery(
              typeof valueOrEvent === "string"
                ? valueOrEvent
                : (valueOrEvent?.target?.value ?? ""),
            )
          }
        />
      </div>

      {/* Compteur */}
      <div className="text-sm font-semibold text-gray-800">
        Résultats ({formattedCount})
      </div>

      {/* Tableau */}
      <DataTable columns={deviceColumns} data={filteredDevices} />
    </div>
  );
}
