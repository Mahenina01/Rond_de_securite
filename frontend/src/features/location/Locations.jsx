import { useCallback, useEffect, useState } from "react";
import Delete from "../../assets/icons/delete.svg?react";
import AddIcon from "../../assets/icons/add.svg?react";
import Input from "../../components/ui/Input";
import IconButton from "../../components/ui/IconButton";
import DataTable from "../../components/ui/DataTable";
import Button from "../../components/ui/Button";
import SignPost from "../../assets/icons/signpost.svg?react";
import Modal from "../../components/ui/Modal";
import locationsService from "../../service/locationsService";
import toast from "react-hot-toast";

const emptyForm = {
  checkpointCode: "",
  name: "",
  agentId: "",
};

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

const Locations = () => {
  const [checkpoints, setCheckpoints] = useState([]);
  const [searchQuery, setSearchQuery] = useState("");

  const [loading, setLoading] = useState(true);
  const [submitting, setSubmitting] = useState(false);
  const [deleting, setDeleting] = useState(false);

  const [isModalOpen, setIsModalOpen] = useState(false);
  const [isDeleteModalOpen, setIsDeleteModalOpen] = useState(false);

  const [formData, setFormData] = useState(emptyForm);
  const [formError, setFormError] = useState("");

  const [editingCheckpoint, setEditingCheckpoint] = useState(null);
  const [selectedCheckpoint, setSelectedCheckpoint] = useState(null);

  // Charger les points de contrôle depuis le backend.
  const loadCheckpoints = useCallback(async () => {
    setLoading(true);

    try {
      const response = await locationsService.getAll();
      const list = getResponseList(response);

      if (!list) {
        throw new Error(
          "Le serveur n'a pas renvoyé une liste valide de points de contrôle.",
        );
      }

      setCheckpoints(list);
    } catch (error) {
      console.error("Erreur de chargement des points de contrôle :", error);

      toast.error(
        getErrorMessage(error, "Impossible de charger les points de contrôle."),
      );
    } finally {
      setLoading(false);
    }
  }, []);

  useEffect(() => {
    loadCheckpoints();
  }, [loadCheckpoints]);

  // Gérer les champs du formulaire.
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

  // Réinitialiser le formulaire.
  const resetForm = () => {
    setFormData(emptyForm);
    setFormError("");
    setEditingCheckpoint(null);
  };

  // Fermer le formulaire.
  const handleCloseModal = () => {
    if (submitting) return;

    setIsModalOpen(false);
    resetForm();
  };

  // Ouvrir le formulaire de création.
  const handleOpenCreate = () => {
    resetForm();
    setIsModalOpen(true);
  };

  // Ouvrir le formulaire de modification.
  const handleEdit = (checkpoint) => {
    setEditingCheckpoint(checkpoint);

    setFormData({
      checkpointCode: checkpoint.checkpointCode ?? "",
      name: checkpoint.name ?? "",
      agentId:
        checkpoint.agentId !== null && checkpoint.agentId !== undefined
          ? String(checkpoint.agentId)
          : "",
    });

    setFormError("");
    setIsModalOpen(true);
  };

  // Créer ou modifier un point de contrôle.
  const handleConfirm = async () => {
    if (submitting) return;

    const checkpointCode = formData.checkpointCode.trim().toUpperCase();

    const name = formData.name.trim();
    const agentId = Number(formData.agentId);

    if (!checkpointCode || !name || !formData.agentId.trim()) {
      setFormError(
        "Le code, le nom et l'identifiant de l'agent sont obligatoires.",
      );
      return;
    }

    if (!Number.isInteger(agentId) || agentId <= 0) {
      setFormError("L'identifiant de l'agent doit être un entier positif.");
      return;
    }

    const duplicateCode = checkpoints.some(
      (checkpoint) =>
        checkpoint.checkpointCode?.toLowerCase() ===
          checkpointCode.toLowerCase() &&
        checkpoint.id !== editingCheckpoint?.id,
    );

    if (duplicateCode) {
      setFormError("Ce code de point de contrôle existe déjà.");
      return;
    }

    // Respecter exactement le format attendu par l'API.
    const payload = {
      checkpointCode,
      name,
      agentId,
    };

    setSubmitting(true);
    setFormError("");

    try {
      if (editingCheckpoint) {
        await locationsService.update(editingCheckpoint.id, payload);

        toast.success("Point de contrôle modifié avec succès.");
      } else {
        await locationsService.create(payload);

        toast.success("Point de contrôle créé avec succès.");
      }

      setIsModalOpen(false);
      resetForm();

      await loadCheckpoints();
    } catch (error) {
      console.error("Erreur d'enregistrement du point de contrôle :", error);

      const message = getErrorMessage(
        error,
        editingCheckpoint
          ? "Impossible de modifier ce point de contrôle."
          : "Impossible de créer ce point de contrôle.",
      );

      setFormError(message);
      toast.error(message);
    } finally {
      setSubmitting(false);
    }
  };

  // Ouvrir la confirmation de suppression.
  const openDeleteModal = (checkpoint) => {
    setSelectedCheckpoint(checkpoint);
    setIsDeleteModalOpen(true);
  };

  // Fermer la confirmation de suppression.
  const closeDeleteModal = () => {
    if (deleting) return;

    setIsDeleteModalOpen(false);
    setSelectedCheckpoint(null);
  };

  // Supprimer un point de contrôle.
  const handleDelete = async () => {
    if (!selectedCheckpoint || deleting) return;

    setDeleting(true);

    try {
      await locationsService.delete(selectedCheckpoint.id);

      setCheckpoints((previous) =>
        previous.filter(
          (checkpoint) => checkpoint.id !== selectedCheckpoint.id,
        ),
      );

      toast.success("Point de contrôle supprimé avec succès.");

      setIsDeleteModalOpen(false);
      setSelectedCheckpoint(null);
    } catch (error) {
      console.error("Erreur de suppression du point de contrôle :", error);

      toast.error(
        getErrorMessage(error, "Impossible de supprimer ce point de contrôle."),
      );
    } finally {
      setDeleting(false);
    }
  };

  // Recherche par code, nom ou identifiant d'agent.
  const normalizedQuery = searchQuery.trim().toLowerCase();

  const filteredCheckpoints = checkpoints.filter((checkpoint) =>
    [
      checkpoint.checkpointCode,
      checkpoint.name,
      checkpoint.agentId,
      checkpoint.isActive ? "actif" : "inactif",
    ].some((field) =>
      String(field ?? "")
        .toLowerCase()
        .includes(normalizedQuery),
    ),
  );

  const formattedCount = String(filteredCheckpoints.length).padStart(2, "0");

  const isFormValid =
    formData.checkpointCode.trim() !== "" &&
    formData.name.trim() !== "" &&
    formData.agentId.trim() !== "" &&
    Number.isInteger(Number(formData.agentId)) &&
    Number(formData.agentId) > 0;

  // Colonnes du tableau.
  const checkpointColumns = [
    {
      header: "Code du point",
      accessor: "checkpointCode",
    },
    {
      header: "Nom du point de contrôle",
      accessor: "name",
    },
    {
      header: "ID de l'agent",
      accessor: "agentId",
    },
    {
      header: "Statut",
      render: (row) => (
        <span
          className={
            row.isActive
              ? "text-sm font-medium text-success"
              : "text-sm font-medium text-danger"
          }
        >
          {row.isActive ? "Actif" : "Inactif"}
        </span>
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
            onClick={() => openDeleteModal(row)}
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
    <div className="flex flex-col gap-6 p-4 rounded-lg bg-surface">
      <div className="flex flex-col items-start justify-between w-full gap-4 sm:flex-row sm:items-center">
        <h1 className="font-sans text-xl text-text-primary">
          Gestion des points de contrôle
        </h1>

        <IconButton
          isButton={true}
          iconSVG={AddIcon}
          text="Nouveau point de contrôle"
          onClick={handleOpenCreate}
        />
      </div>

      {/* Recherche */}
      <div className="w-full sm:w-[320px]">
        <Input
          variant="search"
          placeholder="Rechercher un point de contrôle..."
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

      <div className="text-sm font-semibold text-gray-800">
        Résultats ({formattedCount})
      </div>

      {/* Tableau */}
      {loading ? (
        <p className="py-6 text-sm text-text-secondary">
          Chargement des points de contrôle...
        </p>
      ) : filteredCheckpoints.length > 0 ? (
        <DataTable columns={checkpointColumns} data={filteredCheckpoints} />
      ) : (
        <p className="py-8 text-sm text-center text-text-secondary">
          {searchQuery.trim()
            ? "Aucun point de contrôle ne correspond à votre recherche."
            : "Aucun point de contrôle enregistré."}
        </p>
      )}

      {/* Modal de création et de modification */}
      <Modal
        isOpen={isModalOpen}
        onClose={handleCloseModal}
        title={
          editingCheckpoint
            ? "Modifier un point de contrôle"
            : "Création d'un point de contrôle"
        }
        IconTitle={SignPost}
        footer={
          <>
            <Button
              type="button"
              onClick={handleCloseModal}
              variant="outline"
              disabled={submitting}
            >
              Annuler
            </Button>

            <Button
              type="button"
              onClick={handleConfirm}
              variant="primary"
              disabled={!isFormValid || submitting}
            >
              {submitting
                ? "Enregistrement..."
                : editingCheckpoint
                  ? "Enregistrer"
                  : "Créer"}
            </Button>
          </>
        }
      >
        <div className="flex flex-col gap-3">
          <p className="font-sans text-sm text-text-secondary">
            Enregistrez un point de contrôle et associez-le à un agent
            responsable des rondes de sécurité.
          </p>

          <Input
            variant="default"
            label="Code du point de contrôle"
            placeholder="Ex. : CP-011"
            name="checkpointCode"
            value={formData.checkpointCode}
            onChange={handleChangeFor("checkpointCode")}
            required
          />

          <Input
            variant="default"
            label="Nom du point de contrôle"
            placeholder="Ex. : Parking"
            name="name"
            value={formData.name}
            onChange={handleChangeFor("name")}
            required
          />

          <Input
            variant="default"
            label="ID de l'agent responsable"
            placeholder="Ex. : 1"
            name="agentId"
            inputType="number"
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

      {/* Modal de confirmation de suppression */}
      <Modal
        isOpen={isDeleteModalOpen}
        onClose={closeDeleteModal}
        title="Confirmer la suppression"
        IconTitle={SignPost}
        footer={
          <>
            <Button
              type="button"
              onClick={closeDeleteModal}
              variant="outline"
              disabled={deleting}
            >
              Annuler
            </Button>

            <Button
              type="button"
              onClick={handleDelete}
              variant="primary"
              disabled={deleting}
            >
              {deleting ? "Suppression..." : "Supprimer"}
            </Button>
          </>
        }
      >
        <p className="py-4 text-sm text-text-secondary">
          Êtes-vous sûr de vouloir supprimer le point de contrôle{" "}
          <strong>{selectedCheckpoint?.name ?? ""}</strong> (
          {selectedCheckpoint?.checkpointCode ?? ""}) ? Cette action est
          irréversible.
        </p>
      </Modal>
    </div>
  );
};

export default Locations;
