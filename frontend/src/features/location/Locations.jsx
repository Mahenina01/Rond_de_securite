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
import agentsService from "../../service/agentsService";
import useLanguage from "../../context/useLanguage";
import QRCode from "react-qr-code";

const emptyForm = {
  checkpointCode: "",
  name: "",
  agentName: "",
  isActive: true,
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
  const { t } = useLanguage();
  const [checkpoints, setCheckpoints] = useState([]);
  const [searchQuery, setSearchQuery] = useState("");

  const [loading, setLoading] = useState(true);
  const [submitting, setSubmitting] = useState(false);
  const [deleting, setDeleting] = useState(false);

  const [isModalOpen, setIsModalOpen] = useState(false);
  const [isDeleteModalOpen, setIsDeleteModalOpen] = useState(false);
  const [qrCheckpoint, setQrCheckpoint] = useState(null);

  const [formData, setFormData] = useState(emptyForm);
  const [formError, setFormError] = useState("");

  const [editingCheckpoint, setEditingCheckpoint] = useState(null);
  const [selectedCheckpoint, setSelectedCheckpoint] = useState(null);

  const [agents, setAgents] = useState([]);
  const [agentSuggestionsOpen, setAgentSuggestionsOpen] = useState(false);

  // Charger les agents enregistrés depuis le backend.
  const loadAgents = useCallback(async () => {
    try {
      const response = await agentsService.getAll();
      const list = getResponseList(response);

      if (!list) {
        throw new Error(
          t("Le serveur n'a pas renvoyé une liste valide d'agents."),
        );
      }

      setAgents(list);
    } catch (error) {
      console.error(t("Erreur de chargement des agents :"), error);

      toast.error(
        getErrorMessage(error, t("Impossible de charger la liste des agents.")),
      );
    }
  }, [t]);

  // Charger les points de contrôle depuis le backend.
  const loadCheckpoints = useCallback(async () => {
    try {
      const response = await locationsService.getAll();
      const list = getResponseList(response);

      if (!list) {
        throw new Error(
          t(
            "Le serveur n'a pas renvoyé une liste valide de points de contrôle.",
          ),
        );
      }

      setCheckpoints(list);
    } catch (error) {
      console.error(t("Erreur de chargement des points de contrôle :"), error);

      toast.error(
        getErrorMessage(
          error,
          t("Impossible de charger les points de contrôle."),
        ),
      );
    } finally {
      setLoading(false);
    }
  }, [t]);

  useEffect(() => {
    let cancelled = false;

    void Promise.resolve().then(() => {
      if (cancelled) return;
      loadCheckpoints();
      loadAgents();
    });

    return () => {
      cancelled = true;
    };
  }, [loadCheckpoints, loadAgents]);

  const filteredAgents = agents.filter((agent) => {
    const query = formData.agentName.trim().toLowerCase();

    if (!query) return true;

    return (
      String(agent.name ?? "")
        .toLowerCase()
        .includes(query) ||
      String(agent.agentCode ?? "")
        .toLowerCase()
        .includes(query)
    );
  });

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
    setAgentSuggestionsOpen(false);
  };

  // Fermer le formulaire.
  const handleCloseModal = () => {
    if (submitting) return;

    setIsModalOpen(false);
    setAgentSuggestionsOpen(false);
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
      agentName: checkpoint.agent?.name ?? checkpoint.agentName ?? "",
      isActive: checkpoint.isActive ?? true,
    });

    setFormError("");
    setIsModalOpen(true);
  };

  // Créer ou modifier un point de contrôle.
  const handleConfirm = async () => {
    if (submitting) return;

    const checkpointCode = formData.checkpointCode.trim().toUpperCase();
    const name = formData.name.trim();
    const agentName = formData.agentName.trim();

    if (!checkpointCode || !name || !agentName) {
      setFormError(
        t("Le code, le nom du point et le nom de l'agent sont obligatoires."),
      );
      return;
    }

    const duplicateCode = checkpoints.some(
      (checkpoint) =>
        checkpoint.checkpointCode?.toLowerCase() ===
          checkpointCode.toLowerCase() &&
        checkpoint.id !== editingCheckpoint?.id,
    );

    if (duplicateCode) {
      setFormError(t("Ce code de point de contrôle existe déjà."));
      return;
    }

    // Format attendu par l'API
    const payload = {
      checkpointCode,
      name,
      agentName,
    };

    setSubmitting(true);
    setFormError("");
    let updatingStatus = false;
    let newlyCreatedLocation = null;

    try {
      if (editingCheckpoint) {
        await locationsService.update(editingCheckpoint.id, payload);

        if (formData.isActive !== (editingCheckpoint.isActive ?? true)) {
          updatingStatus = true;
          await locationsService.updateStatus(
            editingCheckpoint.id,
            formData.isActive,
          );
          updatingStatus = false;
        }

        toast.success(t("Point de contrôle modifié avec succès."));
      } else {
        newlyCreatedLocation = await locationsService.create(payload);
        toast.success(t("Point de contrôle créé avec succès."));
      }

      setIsModalOpen(false);
      resetForm();
      if (newlyCreatedLocation) {
        setQrCheckpoint(newlyCreatedLocation);
      }

      setLoading(true);
      await loadCheckpoints();
    } catch (error) {
      console.error(t("Erreur d'enregistrement du point de contrôle :"), error);

      const message = getErrorMessage(
        error,
        updatingStatus
          ? t("Impossible de modifier le statut de ce point de contrôle.")
          : editingCheckpoint
            ? t("Impossible de modifier ce point de contrôle.")
            : t("Impossible de créer ce point de contrôle."),
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

      toast.success(t("Point de contrôle supprimé avec succès."));

      setIsDeleteModalOpen(false);
      setSelectedCheckpoint(null);
    } catch (error) {
      console.error(t("Erreur de suppression du point de contrôle :"), error);

      toast.error(
        getErrorMessage(
          error,
          t("Impossible de supprimer ce point de contrôle."),
        ),
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
      checkpoint.agent?.name,
      checkpoint.agent?.agentCode,
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
    formData.agentName.trim() !== "";

  // Colonnes du tableau.
  const checkpointColumns = [
    {
      header: t("Code du point"),
      accessor: "checkpointCode",
    },
    {
      header: t("Nom du point de contrôle"),
      accessor: "name",
    },
    {
      header: t("Agent responsable"),
      render: (row) => (
        <div className="flex flex-col">
          <span className="text-sm font-medium text-text-primary">
            {row.agent?.name ?? row.agentName ?? t("Non attribué")}
          </span>
          {row.agent?.agentCode && (
            <span className="text-xs text-text-secondary">
              {row.agent.agentCode}
            </span>
          )}
        </div>
      ),
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
            onClick={() => setQrCheckpoint(row)}
            className="text-gray-500 transition-colors hover:text-primary"
            title={t("Afficher le QR code")}
            aria-label={`${t("Afficher le QR code")} ${row.name}`}
          >
            <svg
              xmlns="http://www.w3.org/2000/svg"
              className="w-4 h-4"
              fill="none"
              viewBox="0 0 24 24"
              stroke="currentColor"
              strokeWidth={2}
              aria-hidden="true"
            >
              <path
                strokeLinecap="round"
                strokeLinejoin="round"
                d="M3 3h7v7H3zM14 3h7v7h-7zM3 14h7v7H3zM14 14h3v3h-3zM20 14v2m-3 3h2m2 0v2m-6-4v3"
              />
            </svg>
          </button>
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
      <div className="flex flex-col items-start justify-between w-full gap-4 sm:flex-row sm:items-center">
        <h1 className="font-sans text-xl text-text-primary">
          {t("Gestion des points de contrôle")}
        </h1>

        <IconButton
          isButton={true}
          iconSVG={AddIcon}
          text={t("Nouveau point de contrôle")}
          onClick={handleOpenCreate}
        />
      </div>

      {/* Recherche */}
      <div className="w-full sm:w-[320px]">
        <Input
          variant="search"
          placeholder={t("Rechercher un point de contrôle...")}
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
        {t("Résultats")} ({formattedCount})
      </div>

      {/* Tableau */}
      {loading ? (
        <p className="py-6 text-sm text-text-secondary">
          {t("Chargement des points de contrôle...")}
        </p>
      ) : filteredCheckpoints.length > 0 ? (
        <DataTable columns={checkpointColumns} data={filteredCheckpoints} />
      ) : (
        <p className="py-8 text-sm text-center text-text-secondary">
          {searchQuery.trim()
            ? t("Aucun point de contrôle ne correspond à votre recherche.")
            : t("Aucun point de contrôle enregistré.")}
        </p>
      )}

      {/* Modal de création et de modification */}
      <Modal
        isOpen={isModalOpen}
        onClose={handleCloseModal}
        title={
          editingCheckpoint
            ? t("Modifier un point de contrôle")
            : t("Création d'un point de contrôle")
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
                : editingCheckpoint
                  ? t("Enregistrer")
                  : t("Créer")}
            </Button>
          </>
        }
      >
        <div className="flex flex-col gap-3">
          <p className="font-sans text-sm text-text-secondary">
            {t(
              "Enregistrez un point de contrôle et associez-le à un agent responsable des rondes de sécurité.",
            )}
          </p>

          <Input
            variant="default"
            label={t("Code du point de contrôle")}
            placeholder={t("Ex. : CP-011")}
            name="checkpointCode"
            value={formData.checkpointCode}
            onChange={handleChangeFor("checkpointCode")}
            required
          />

          <Input
            variant="default"
            label={t("Nom du point de contrôle")}
            placeholder={t("Ex. : Parking")}
            name="name"
            value={formData.name}
            onChange={handleChangeFor("name")}
            required
          />

          <div className="relative">
            <Input
              variant="default"
              label={t("Nom de l'agent responsable")}
              placeholder={t("Rechercher par nom ou code agent...")}
              name="agentName"
              value={formData.agentName}
              onChange={(valueOrEvent) => {
                handleChangeFor("agentName")(valueOrEvent);
                setAgentSuggestionsOpen(true);
              }}
              onFocus={() => setAgentSuggestionsOpen(true)}
              required
            />

            {agentSuggestionsOpen && (
              <div className="absolute z-50 w-full mt-1 overflow-y-auto bg-white border border-gray-200 rounded-lg shadow-lg max-h-52">
                {filteredAgents.length > 0 ? (
                  filteredAgents.map((agent) => (
                    <button
                      key={agent.id ?? agent.agentCode}
                      type="button"
                      onClick={() => {
                        setFormData((previous) => ({
                          ...previous,
                          agentName: agent.name ?? "",
                        }));

                        setAgentSuggestionsOpen(false);
                        setFormError("");
                      }}
                      className="flex flex-col items-start w-full gap-1 px-3 py-2 text-left transition-colors border-b border-gray-100 last:border-b-0 hover:bg-gray-50"
                    >
                      <span className="text-sm font-medium text-text-primary">
                        {agent.name}
                      </span>

                      <span className="text-xs text-text-secondary">
                        {t("Code agent : ")}
                        {agent.agentCode}
                      </span>
                    </button>
                  ))
                ) : (
                  <p className="px-3 py-3 text-sm text-text-secondary">
                    {t("Aucun agent correspondant.")}
                  </p>
                )}
              </div>
            )}
          </div>

          {editingCheckpoint && (
            <div className="flex items-center justify-between gap-4 py-2">
              <div>
                <p className="text-sm font-medium text-text-primary">
                  {t("Statut du point de contrôle")}
                </p>
                <p className="text-sm text-text-secondary">
                  {formData.isActive ? t("Actif") : t("Inactif")}
                </p>
              </div>
              <button
                type="button"
                role="switch"
                aria-checked={formData.isActive}
                aria-label={
                  formData.isActive
                    ? t("Désactiver le point de contrôle")
                    : t("Activer le point de contrôle")
                }
                disabled={submitting}
                onClick={() =>
                  setFormData((previous) => ({
                    ...previous,
                    isActive: !previous.isActive,
                  }))
                }
                className={`relative h-6 w-12 shrink-0 rounded-full transition-colors focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-primary disabled:cursor-not-allowed disabled:opacity-60 ${
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

      {/* Modal de confirmation de suppression */}
      <Modal
        isOpen={isDeleteModalOpen}
        onClose={closeDeleteModal}
        title={t("Confirmer la suppression")}
        IconTitle={SignPost}
        footer={
          <>
            <Button
              type="button"
              onClick={closeDeleteModal}
              variant="outline"
              disabled={deleting}
            >
              {t("Annuler")}
            </Button>

            <button
              type="button"
              onClick={handleDelete}
              disabled={deleting}
              className="px-4 py-2 text-sm text-white rounded-lg bg-danger hover:bg-red-700 disabled:cursor-not-allowed disabled:opacity-50"
            >
              {deleting ? t("Suppression...") : t("Supprimer")}
            </button>
          </>
        }
      >
        <p className="py-4 text-sm text-text-secondary">
          {t("Êtes-vous sûr de vouloir supprimer le point de contrôle")}{" "}
          <strong>{selectedCheckpoint?.name ?? ""}</strong> (
          {selectedCheckpoint?.checkpointCode ?? ""})?{" "}
          {t("Cette action est irréversible.")}
        </p>
      </Modal>

      <Modal
        isOpen={Boolean(qrCheckpoint)}
        onClose={() => setQrCheckpoint(null)}
        title={t("QR code du point de contrôle")}
        IconTitle={SignPost}
        footer={
          <Button
            type="button"
            variant="outline"
            onClick={() => setQrCheckpoint(null)}
          >
            {t("Fermer")}
          </Button>
        }
      >
        {qrCheckpoint && (
          <div className="flex flex-col items-center gap-4 text-center">
            <p className="text-sm text-text-secondary">
              {t(
                "Scannez ce QR code pour lire les données du point de contrôle.",
              )}
            </p>

            <div className="p-4 bg-white border border-gray-200 rounded-lg">
              <QRCode
                value={JSON.stringify(qrCheckpoint)}
                size={220}
                level="M"
                title={t("QR code du point de contrôle")}
              />
            </div>
          </div>
        )}
      </Modal>
    </div>
  );
};

export default Locations;
