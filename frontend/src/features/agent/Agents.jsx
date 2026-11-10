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
import useLanguage from "../../context/useLanguage";

const emptyForm = {
  agentCode: "",
  name: "",
  phone: "",
  isActive: true,
};

const getErrorMessage = (error, fallback) =>
  error.response?.data?.message || error.response?.data?.detail || fallback;

const Agents = () => {
  const { t } = useLanguage();
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
        throw new Error(t("Le backend n'a pas renvoyé une liste d'agents."));
      }

      setAgents(list);
    } catch (error) {
      console.error(t("Erreur de chargement des agents :"), error);
      toast.error(getErrorMessage(error, t("Impossible de charger les agents.")));
    } finally {
      setLoading(false);
    }
  }, [t]);

  useEffect(() => {
    let cancelled = false;

    void Promise.resolve().then(() => {
      if (!cancelled) return loadAgents();
    });

    return () => {
      cancelled = true;
    };
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
      isActive: agent.isActive ?? true,
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
      setFormError(t("Le matricule et le nom complet sont obligatoires."));
      return;
    }

    const duplicate = agents.some(
      (agent) =>
        agent.agentCode?.toLowerCase() === payload.agentCode.toLowerCase() &&
        agent.id !== editingAgent?.id,
    );

    if (duplicate) {
      setFormError(t("Ce matricule existe déjà."));
      return;
    }

    setSubmitting(true);
    let updatingStatus = false;

    try {
      if (editingAgent) {
        await agentsService.update(editingAgent.id, payload);

        if (formData.isActive !== (editingAgent.isActive ?? true)) {
          updatingStatus = true;
          await agentsService.updateStatus(editingAgent.id, formData.isActive);
          updatingStatus = false;
        }

        toast.success(t("Agent modifié avec succès."));
      } else {
        await agentsService.create({
          ...payload,
          isActive: true,
        });

        toast.success(t("Agent créé avec succès."));
      }

      setIsModalOpen(false);
      resetForm();
      setLoading(true);
      await loadAgents();
    } catch (error) {
      console.error(t("Erreur d'enregistrement de l'agent :"), error);

      const message = getErrorMessage(
        error,
        updatingStatus
          ? t("Impossible de modifier le statut de cet agent.")
          : editingAgent
            ? t("Impossible de modifier cet agent.")
            : t("Impossible de créer cet agent."),
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

      toast.success(t("Agent supprimé avec succès."));
      setIsDeleteModalOpen(false);
      setSelectedAgent(null);
    } catch (error) {
      console.error(t("Erreur de suppression :"), error);

      toast.error(
        getErrorMessage(error, t("Impossible de supprimer cet agent.")),
      );
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
      header: t("ID Agent"),
      accessor: "agentCode",
    },
    {
      header: t("Nom complet"),
      accessor: "name",
    },
    {
      header: t("Téléphone"),
      accessor: "phone",
    },
    {
      header: t("Statut"),
      render: (row) => (
        <span
          className={
            row.isActive
              ? "text-sm font-medium text-success"
              : "text-sm font-medium text-danger"
          }
        >
          {row.isActive ? t("Actif") : t("Inactif")}
        </span>
      ),
    },
    {
      header: t("Actions"),
      align: "right",
      render: (row) => (
        <div className="flex items-center justify-end gap-3">
          <button
            type="button"
            onClick={() => handleEdit(row)}
            className="text-gray-500 transition-colors hover:text-blue-600"
            title={t("Modifier")}
            aria-label={`${t("Modifier")} ${row.name}`}
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
            title={t("Supprimer")}
            aria-label={`${t("Supprimer")} ${row.name}`}
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
        {t("Gestion des Agents")}
      </h1>

      <div className="flex flex-col items-start justify-between w-full gap-4 sm:flex-row sm:items-center">
        <div className="w-full sm:w-[320px]">
          <Input
            variant="search"
            placeholder={t("Recherche...")}
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
        {t("Résultats")} ({formattedCount})
      </div>

      {loading ? (
        <p className="py-6 text-sm text-text-secondary">
          {t("Chargement des agents...")}
        </p>
      ) : (
        <DataTable columns={agentColumns} data={filteredAgents} />
      )}

      <Modal
        isOpen={isModalOpen}
        onClose={handleCloseModal}
        title={
          editingAgent
            ? t("Modifier un agent")
            : t("Ajout d'un nouvel agent")
        }
        IconTitle={UserIcon}
        footer={
          <>
            <Button
              type="button"
              onClick={handleCloseModal}
              variant="outline"
              disabled={submitting}
            >
              {t("Annuler")}
            </Button>

            <Button
              type="button"
              onClick={handleConfirm}
              variant="primary"
              disabled={!isFormValid || submitting}
            >
              {submitting
                ? t("Enregistrement...")
                : editingAgent
                  ? t("Enregistrer")
                  : t("Ajouter")}
            </Button>
          </>
        }
      >
        <div className="flex flex-col gap-3">
          <p className="font-sans text-sm text-text-secondary">
            {t(
              "Renseignez les informations nécessaires pour créer ou modifier la fiche de l'agent.",
            )}
          </p>

          <Input
            variant="default"
            label={t("Matricule de l'agent")}
            placeholder={t("Ex. : AG-061")}
            name="agentCode"
            value={formData.agentCode}
            onChange={handleChangeFor("agentCode")}
            required
          />

          <Input
            variant="default"
            label={t("Nom complet")}
            placeholder={t("Ex. : Rakoto Jean")}
            name="name"
            value={formData.name}
            onChange={handleChangeFor("name")}
            required
          />

          <Input
            variant="default"
            label={t("Téléphone (facultatif)")}
            placeholder={t("Ex. : 0341234567")}
            name="phone"
            value={formData.phone}
            onChange={handleChangeFor("phone")}
          />

          {editingAgent && (
            <div className="flex items-center justify-between gap-4 py-2">
              <div>
                <p className="text-sm font-medium text-text-primary">
                  {t("Statut de l’agent")}
                </p>
                <p className="text-sm text-text-secondary">
                  {formData.isActive ? t("Actif") : t("Inactif")}
                </p>
              </div>
              <button
                type="button"
                role="switch"
                aria-checked={formData.isActive}
                disabled={submitting}
                aria-label={
                  formData.isActive
                    ? t("Désactiver l’agent")
                    : t("Activer l’agent")
                }
                onClick={() =>
                  setFormData((previous) => ({
                    ...previous,
                    isActive: !previous.isActive,
                  }))
                }
                className={`relative h-6 w-12 shrink-0 rounded-full transition-colors focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-primary ${
                  formData.isActive ? "bg-primary" : "bg-gray-300"
                }`}
              >
                <span
                  className={`absolute top-1 h-4 w-4 rounded-full bg-white shadow-sm transition-all ${
                    formData.isActive ? "left-7" : "left-1"
                  }`}
                />
              </button>
            </div>
          )}

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
        title={t("Confirmer la suppression")}
        IconTitle={UserIcon}
        footer={
          <>
            <button
              type="button"
              onClick={closeDeleteModal}
              disabled={deleting}
              className="px-4 py-2 text-sm border border-gray-300 rounded-lg disabled:cursor-not-allowed disabled:opacity-50"
            >
              {t("Annuler")}
            </button>

            <button
              type="button"
              onClick={handleDelete}
              disabled={deleting}
              className="px-4 py-2 text-sm text-white rounded-lg bg-danger disabled:cursor-not-allowed disabled:opacity-50"
            >
              {deleting ? t("Suppression...") : t("Supprimer")}
            </button>
          </>
        }
      >
        <p className="py-4 text-sm text-text-secondary">
          {t("Êtes-vous sûr de vouloir supprimer ")}{" "}
          {selectedAgent?.name
            ? `${t("l'agent")} ${selectedAgent.name}`
            : t("cet élément")}{" "}
          ? {t("Cette action est irréversible.")}
        </p>
      </Modal>
    </div>
  );
};

export default Agents;
