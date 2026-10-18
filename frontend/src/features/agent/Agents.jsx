import { useCallback, useEffect, useState } from "react";
import Input from "../../components/ui/Input";
import IconButton from "../../components/ui/IconButton";
import AddIcon from "../../assets/icons/add.svg?react";
import Delete from "../../assets/icons/delete.svg?react";
import DataTable from "../../components/ui/DataTable";
import Modal from "../../components/ui/Modal";
import Button from "../../components/ui/Button";
import UserIcon from "../../assets/icons/user-round-plus.svg?react";
import agentsService from "../../service/agentsService";
import toast from "react-hot-toast";

const emptyForm = {
  agentCode: "",
  name: "",
  phone: "",
};

const getErrorMessage = (error, fallback) =>
  error.response?.data?.message || error.response?.data?.detail || fallback;

const Agents = () => {
  const [agents, setAgents] = useState([]);
  const [searchQuery, setSearchQuery] = useState("");
  const [isModalOpen, setIsModalOpen] = useState(false);
  const [formData, setFormData] = useState(emptyForm);
  const [formError, setFormError] = useState("");
  const [loading, setLoading] = useState(true);
  const [submitting, setSubmitting] = useState(false);
  const [editingAgent, setEditingAgent] = useState(null);

  const [isDeleteModalOpen, setIsDeleteModalOpen] = useState(false);
  const [selectedAgent, setSelectedAgent] = useState(null);
  const [deleting, setDeleting] = useState(false);

  // Charger les agents depuis le backend
  const loadAgents = useCallback(async () => {
    setLoading(true);

    try {
      const data = await agentsService.getAll();

      // Prend en charge une liste directe ou une réponse contenant content/data.
      const list = Array.isArray(data)
        ? data
        : Array.isArray(data?.content)
          ? data.content
          : Array.isArray(data?.data)
            ? data.data
            : null;

      if (!list) {
        throw new Error("Le backend n'a pas renvoyé une liste d'agents.");
      }

      setAgents(list);
    } catch (error) {
      console.error("Erreur de chargement des agents :", error);
      toast.error(getErrorMessage(error, "Impossible de charger les agents."));
    } finally {
      setLoading(false);
    }
  }, []);

  useEffect(() => {
    loadAgents();
  }, [loadAgents]);

  const resetForm = () => {
    setFormData(emptyForm);
    setFormError("");
    setEditingAgent(null);
  };

  const handleCloseModal = () => {
    if (submitting) return;

    setIsModalOpen(false);
    resetForm();
  };

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

  // Ouvrir le formulaire en mode création
  const handleOpenCreate = () => {
    resetForm();
    setIsModalOpen(true);
  };

  // Ouvrir le formulaire en mode modification
  const handleEdit = (agent) => {
    setEditingAgent(agent);

    setFormData({
      agentCode: agent.agentCode ?? "",
      name: agent.name ?? "",
      phone: agent.phone ?? "",
    });

    setFormError("");
    setIsModalOpen(true);
  };

  // Créer ou modifier un agent
  const handleConfirm = async () => {
    const payload = {
      agentCode: formData.agentCode.trim().toUpperCase(),
      name: formData.name.trim(),
      phone: formData.phone.trim(),
    };

    if (!payload.agentCode || !payload.name) {
      setFormError("Le matricule et le nom complet sont obligatoires.");
      return;
    }

    const duplicate = agents.some(
      (agent) =>
        agent.agentCode?.toLowerCase() === payload.agentCode.toLowerCase() &&
        agent.id !== editingAgent?.id,
    );

    if (duplicate) {
      setFormError("Ce matricule existe déjà.");
      return;
    }

    setSubmitting(true);

    try {
      if (editingAgent) {
        await agentsService.update(editingAgent.id, {
          ...payload,
          isActive: editingAgent.isActive,
        });

        toast.success("Agent modifié avec succès.");
      } else {
        await agentsService.create({
          ...payload,
          isActive: true,
        });

        toast.success("Agent créé avec succès.");
      }

      setIsModalOpen(false);
      resetForm();
      await loadAgents();
    } catch (error) {
      console.error("Erreur d'enregistrement de l'agent :", error);

      const message = getErrorMessage(
        error,
        editingAgent
          ? "Impossible de modifier cet agent."
          : "Impossible de créer cet agent.",
      );

      setFormError(message);
      toast.error(message);
    } finally {
      setSubmitting(false);
    }
  };

  // Supprimer un agent
  const openDeleteModal = (agent) => {
    setSelectedAgent(agent);
    setIsDeleteModalOpen(true);
  };

  const closeDeleteModal = () => {
    if (deleting) return;

    setIsDeleteModalOpen(false);
    setSelectedAgent(null);
  };

  const handleDelete = async () => {
    if (!selectedAgent || deleting) return;

    setDeleting(true);

    try {
      await agentsService.delete(selectedAgent.id);

      setAgents((previous) =>
        previous.filter((agent) => agent.id !== selectedAgent.id),
      );

      toast.success("Agent supprimé avec succès.");
      setIsDeleteModalOpen(false);
      setSelectedAgent(null);
    } catch (error) {
      console.error("Erreur de suppression :", error);

      toast.error(getErrorMessage(error, "Impossible de supprimer cet agent."));
    } finally {
      setDeleting(false);
    }
  };

  const normalizedQuery = searchQuery.trim().toLowerCase();

  const filteredAgents = agents.filter((agent) =>
    [
      agent.agentCode,
      agent.name,
      agent.phone,
      agent.isActive ? "actif" : "inactif",
    ].some((field) =>
      String(field ?? "")
        .toLowerCase()
        .includes(normalizedQuery),
    ),
  );

  const formattedCount = String(filteredAgents.length).padStart(2, "0");

  const isFormValid =
    formData.agentCode.trim().length > 0 && formData.name.trim().length > 0;

  const agentColumns = [
    {
      header: "ID Agent",
      accessor: "agentCode",
    },
    {
      header: "Nom complet",
      accessor: "name",
    },
    {
      header: "Téléphone",
      accessor: "phone",
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
      <h1 className="font-sans text-xl text-text-primary">
        Gestion des Agents
      </h1>

      <div className="flex flex-col items-start justify-between w-full gap-4 sm:flex-row sm:items-center">
        <div className="w-full sm:w-[320px]">
          <Input
            variant="search"
            placeholder="Recherche..."
            name="searchQuery"
            value={searchQuery}
            onChange={(value) => {
              setSearchQuery(
                typeof value === "string"
                  ? value
                  : (value?.target?.value ?? ""),
              );
            }}
          />
        </div>

        <IconButton
          isButton={true}
          iconSVG={AddIcon}
          text="Nouvel agent"
          onClick={handleOpenCreate}
        />
      </div>

      <div className="text-sm font-semibold text-gray-800">
        Résultats ({formattedCount})
      </div>

      {loading ? (
        <p className="py-6 text-sm text-text-secondary">
          Chargement des agents...
        </p>
      ) : (
        <DataTable columns={agentColumns} data={filteredAgents} />
      )}

      <Modal
        isOpen={isModalOpen}
        onClose={handleCloseModal}
        title={editingAgent ? "Modifier un agent" : "Ajout d'un nouvel agent"}
        IconTitle={UserIcon}
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
                : editingAgent
                  ? "Enregistrer"
                  : "Ajouter"}
            </Button>
          </>
        }
      >
        <div className="flex flex-col gap-3">
          <p className="font-sans text-sm text-text-secondary">
            Renseignez les informations nécessaires pour créer ou modifier la
            fiche de l'agent.
          </p>

          <Input
            variant="default"
            label="Matricule de l'agent"
            placeholder="Ex. : AG-061"
            name="agentCode"
            value={formData.agentCode}
            onChange={handleChangeFor("agentCode")}
            required
          />

          <Input
            variant="default"
            label="Nom complet"
            placeholder="Ex. : Rakoto Jean"
            name="name"
            value={formData.name}
            onChange={handleChangeFor("name")}
            required
          />

          <Input
            variant="default"
            label="Téléphone (facultatif)"
            placeholder="Ex. : 0341234567"
            name="phone"
            value={formData.phone}
            onChange={handleChangeFor("phone")}
          />

          {formError && (
            <p className="text-sm text-danger" role="alert">
              {formError}
            </p>
          )}
        </div>
      </Modal>

      <Modal
        isOpen={isDeleteModalOpen}
        onClose={closeDeleteModal}
        title={"Confirmer la suppression"}
        IconTitle={UserIcon}
        footer={
          <>
            <button
              type="button"
              onClick={closeDeleteModal}
              disabled={deleting}
              className="px-4 py-2 text-sm border border-gray-300 rounded-lg disabled:cursor-not-allowed disabled:opacity-50"
            >
              Annuler
            </button>

            <button
              type="button"
              onClick={handleDelete}
              disabled={deleting}
              className="px-4 py-2 text-sm text-white rounded-lg bg-danger disabled:cursor-not-allowed disabled:opacity-50"
            >
              {deleting ? "Suppression..." : "Supprimer"}
            </button>
          </>
        }
      >
        <p className="py-4 text-sm text-text-secondary">
          Êtes-vous sûr de vouloir supprimer{" "}
          {selectedAgent?.name
            ? `l'agent ${selectedAgent?.name}`
            : "cet élément"}{" "}
          ? Cette action est irréversible.
        </p>
      </Modal>
    </div>
  );
};

export default Agents;
