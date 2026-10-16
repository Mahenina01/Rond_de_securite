import { useState } from "react";
import Delete from "../../assets/icons/delete.svg?react";
import AddIcon from "../../assets/icons/add.svg?react";
import Input from "../../components/ui/Input";
import IconButton from "../../components/ui/IconButton";
import DataTable from "../../components/ui/DataTable";
import Button from "../../components/ui/Button";
import SignPost from "../../assets/icons/signpost.svg?react";
import Modal from "../../components/ui/Modal";

const initialCheckpoints = [
  { id: 1, idCheckpoint: "CP-001", nameCheckpoint: "Salle 1", agent: "AG-024" },
  { id: 2, idCheckpoint: "CP-002", nameCheckpoint: "Salle 2", agent: "AG-043" },
  { id: 3, idCheckpoint: "CP-003", nameCheckpoint: "Salle 3", agent: "AG-054" },
  { id: 4, idCheckpoint: "CP-004", nameCheckpoint: "Salle 4", agent: "AG-055" },
  {
    id: 5,
    idCheckpoint: "CP-005",
    nameCheckpoint: "Salle 5",
    agent: "AG-0541",
  },
  { id: 6, idCheckpoint: "CP-006", nameCheckpoint: "Salle 6", agent: "AG-056" },
  { id: 7, idCheckpoint: "CP-007", nameCheckpoint: "Salle 7", agent: "AG-057" },
  { id: 8, idCheckpoint: "CP-008", nameCheckpoint: "Salle 8", agent: "AG-058" },
  { id: 9, idCheckpoint: "CP-009", nameCheckpoint: "Salle 9", agent: "AG-059" },
  {
    id: 10,
    idCheckpoint: "CP-010",
    nameCheckpoint: "Salle 10",
    agent: "AG-060",
  },
];

const emptyForm = {
  idCheckpoint: "",
  nameCheckpoint: "",
  agent: "",
};

const Locations = () => {
  const [isModalOpen, setIsModalOpen] = useState(false);
  const [searchQuery, setSearchQuery] = useState("");
  const [checkpoints, setCheckpoints] = useState(initialCheckpoints);
  const [formData, setFormData] = useState(emptyForm);
  const [formError, setFormError] = useState("");

  // Gestion des champs du formulaire.
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

  const resetForm = () => {
    setFormData(emptyForm);
    setFormError("");
  };

  const handleCloseModal = () => {
    setIsModalOpen(false);
    resetForm();
  };

  // Création d'un point de contrôle.
  const handleConfirm = () => {
    const idCheckpoint = formData.idCheckpoint.trim().toUpperCase();
    const nameCheckpoint = formData.nameCheckpoint.trim();
    const agent = formData.agent.trim().toUpperCase();

    if (!idCheckpoint || !nameCheckpoint) {
      setFormError("Le code et le nom du point de contrôle sont obligatoires.");
      return;
    }

    const duplicateCode = checkpoints.some(
      (checkpoint) =>
        checkpoint.idCheckpoint.toLowerCase() === idCheckpoint.toLowerCase(),
    );

    if (duplicateCode) {
      setFormError("Ce code de point de contrôle existe déjà.");
      return;
    }

    const newCheckpoint = {
      id:
        checkpoints.length > 0
          ? Math.max(...checkpoints.map((checkpoint) => checkpoint.id)) + 1
          : 1,
      idCheckpoint,
      nameCheckpoint,
      agent: agent || "Non assigné",
    };

    setCheckpoints((previous) => [...previous, newCheckpoint]);
    setSearchQuery("");
    handleCloseModal();
  };

  const handleDelete = (id) => {
    setCheckpoints((previous) =>
      previous.filter((checkpoint) => checkpoint.id !== id),
    );
  };

  const handleEdit = (checkpoint) => {
    alert(`Modification du point de contrôle : ${checkpoint.nameCheckpoint}`);
  };

  const normalizedQuery = searchQuery.trim().toLowerCase();

  const filteredCheckpoints = checkpoints.filter((checkpoint) =>
    [checkpoint.idCheckpoint, checkpoint.nameCheckpoint, checkpoint.agent].some(
      (field) =>
        String(field ?? "")
          .toLowerCase()
          .includes(normalizedQuery),
    ),
  );

  const formattedCount = String(filteredCheckpoints.length).padStart(2, "0");

  const isFormValid =
    formData.idCheckpoint.trim().length > 0 &&
    formData.nameCheckpoint.trim().length > 0;

  const checkpointColumns = [
    { header: "ID Checkpoint", accessor: "idCheckpoint" },
    { header: "Nom point de contrôle", accessor: "nameCheckpoint" },
    { header: "Responsable", accessor: "agent" },
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

      <div className="flex flex-row items-center justify-between w-full gap-4">
        <div className="w-full sm:w-[320px]">
          <Input
            variant="search"
            placeholder="Recherche..."
            name="searchQuery"
            value={searchQuery}
            onChange={(value) =>
              setSearchQuery(
                typeof value === "string" ? value : value.target.value,
              )
            }
          />
        </div>

        <IconButton
          isButton={true}
          iconSVG={AddIcon}
          text="Nouveau point de contrôle"
          onClick={() => setIsModalOpen(true)}
        />

        <Modal
          isOpen={isModalOpen}
          onClose={handleCloseModal}
          title="Création d'un point de contrôle"
          IconTitle={SignPost}
          footer={
            <>
              <Button
                type="button"
                onClick={handleCloseModal}
                variant="outline"
              >
                Annuler
              </Button>

              <Button
                type="button"
                onClick={handleConfirm}
                variant="primary"
                disabled={!isFormValid}
              >
                Créer
              </Button>
            </>
          }
        >
          <div className="flex flex-col gap-3">
            <p className="font-sans text-sm text-text-secondary">
              Enregistrez un emplacement équipé d'une carte RFID pour permettre
              le suivi des rondes de sécurité.
            </p>

            <Input
              variant="default"
              label="Code du point de contrôle"
              placeholder="Ex. : CP-011"
              name="idCheckpoint"
              value={formData.idCheckpoint}
              onChange={handleChangeFor("idCheckpoint")}
              required={true}
            />

            <Input
              variant="default"
              label="Nom du point de contrôle"
              placeholder="Ex. : Parking"
              name="nameCheckpoint"
              value={formData.nameCheckpoint}
              onChange={handleChangeFor("nameCheckpoint")}
              required={true}
            />

            <Input
              variant="default"
              label="Agent responsable (facultatif)"
              placeholder="Ex. : AG-024"
              name="agent"
              value={formData.agent}
              onChange={handleChangeFor("agent")}
              required={false}
            />

            {formError && (
              <p className="text-sm text-danger" role="alert">
                {formError}
              </p>
            )}
          </div>
        </Modal>
      </div>

      <div className="text-sm font-semibold text-gray-800">
        Résultats ({formattedCount})
      </div>

      <DataTable columns={checkpointColumns} data={filteredCheckpoints} />
    </div>
  );
};

export default Locations;
