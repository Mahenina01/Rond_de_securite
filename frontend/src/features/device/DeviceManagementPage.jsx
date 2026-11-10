import { useCallback, useEffect, useState } from "react";
import Button from "../../components/ui/Button";
import Input from "../../components/ui/Input";
import DataTable from "../../components/ui/DataTable";
import Delete from "../../assets/icons/delete.svg?react";
import Modal from "../../components/ui/Modal";
import DeviceIcon from "../../assets/icons/device-out.svg?react";
import devicesService from "../../service/devciesService";
import toast from "react-hot-toast";
import agentsService from "../../service/agentsService";
import useLanguage from "../../context/useLanguage";

const emptyForm = {
  deviceCode: "",
  nameAgent: "",
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

export default function DeviceManagementPage() {
  const { t } = useLanguage();
  const [devices, setDevices] = useState([]);
  const [searchQuery, setSearchQuery] = useState("");

  const [loading, setLoading] = useState(true);
  const [submitting, setSubmitting] = useState(false);
  const [deleting, setDeleting] = useState(false);

  const [isModalOpen, setIsModalOpen] = useState(false);
  const [isDeleteModalOpen, setIsDeleteModalOpen] = useState(false);

  const [formData, setFormData] = useState(emptyForm);
  const [formError, setFormError] = useState("");

  const [editingDevice, setEditingDevice] = useState(null);
  const [selectedDevice, setSelectedDevice] = useState(null);

  const [agents, setAgents] = useState([]);
  const [agentSuggestionsOpen, setAgentSuggestionsOpen] = useState(false);

  //Charger les listes des agent
  const loadAgents = useCallback(async () => {
    try {
      const response = await agentsService.getAll();
      const list = getResponseList(response);

      if (!list) {
        throw new Error(t("La liste des agents reçue est invalide."));
      }

      setAgents(list);
    } catch (error) {
      console.error(t("Erreur de chargement des agents :"), error);
      toast.error(getErrorMessage(error, t("Impossible de charger les agents.")));
    }
  }, [t]);

  // Charger les appareils depuis le backend
  const loadDevices = useCallback(async () => {
    try {
      const response = await devicesService.getAll();
      const list = getResponseList(response);

      if (!list) {
        throw new Error(
          t("Le serveur n'a pas renvoyé une liste valide d'appareils."),
        );
      }

      setDevices(list);
    } catch (error) {
      console.error(t("Erreur de chargement des appareils :"), error);

      toast.error(
        getErrorMessage(error, t("Impossible de charger les appareils.")),
      );
    } finally {
      setLoading(false);
    }
  }, [t]);

  useEffect(() => {
    let cancelled = false;

    void Promise.resolve().then(() => {
      if (cancelled) return;
      loadDevices();
      loadAgents();
    });

    return () => {
      cancelled = true;
    };
  }, [loadDevices, loadAgents]);

  // Réinitialiser le formulaire
  const resetForm = () => {
    setFormData(emptyForm);
    setFormError("");
    setEditingDevice(null);
    setAgentSuggestionsOpen(false);
  };

  // Fermer le formulaire
  const handleCloseModal = () => {
    if (submitting) return;

    setIsModalOpen(false);
    setAgentSuggestionsOpen(false);
    resetForm();
  };

  // Gérer les champs du formulaire
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

  // Ouvrir le formulaire de création
  const handleOpenCreate = () => {
    resetForm();
    setIsModalOpen(true);
  };

  // Ouvrir le formulaire de modification
  const handleEdit = (device) => {
    setEditingDevice(device);

    setFormData({
      deviceCode: device.deviceCode ?? "",
      nameAgent: device.nameAgent ?? "",
      isActive: device.isActive ?? true,
    });

    setFormError("");
    setIsModalOpen(true);
  };

  // Créer ou modifier un appareil
  const handleConfirm = async () => {
    if (submitting) return;

    const payload = {
      deviceCode: formData.deviceCode.trim().toUpperCase(),
      nameAgent: formData.nameAgent.trim(),
    };

    if (!payload.deviceCode || !payload.nameAgent) {
      setFormError(
        t("Le code de l'appareil et le nom de l'agent sont obligatoires."),
      );
      return;
    }

    // Vérifier les doublons dans la liste chargée
    const duplicate = devices.some(
      (device) =>
        device.deviceCode?.toLowerCase() === payload.deviceCode.toLowerCase() &&
        device.id !== editingDevice?.id,
    );

    if (duplicate) {
      setFormError(t("Ce code appareil existe déjà."));
      return;
    }

    setSubmitting(true);
    setFormError("");
    let updatingStatus = false;

    try {
      if (editingDevice) {
        await devicesService.update(editingDevice.id, payload);

        if (formData.isActive !== (editingDevice.isActive ?? true)) {
          updatingStatus = true;
          await devicesService.updateStatus(editingDevice.id, formData.isActive);
          updatingStatus = false;
        }

        toast.success(t("Appareil modifié avec succès."));
      } else {
        await devicesService.create(payload);
        toast.success(t("Appareil créé avec succès."));
      }

      setIsModalOpen(false);
      resetForm();

      setLoading(true);
      await loadDevices();
    } catch (error) {
      console.error(t("Erreur d'enregistrement de l'appareil :"), error);

      const message = getErrorMessage(
        error,
        updatingStatus
          ? t("Impossible de modifier le statut de cet appareil.")
          : editingDevice
            ? t("Impossible de modifier cet appareil.")
            : t("Impossible de créer cet appareil."),
      );

      setFormError(message);
      toast.error(message);
    } finally {
      setSubmitting(false);
    }
  };

  // Ouvrir la confirmation de suppression
  const openDeleteModal = (device) => {
    setSelectedDevice(device);
    setIsDeleteModalOpen(true);
  };

  // Fermer la confirmation de suppression
  const closeDeleteModal = () => {
    if (deleting) return;

    setIsDeleteModalOpen(false);
    setSelectedDevice(null);
  };

  // Supprimer un appareil
  const handleDelete = async () => {
    if (!selectedDevice || deleting) return;

    setDeleting(true);

    try {
      await devicesService.delete(selectedDevice.id);

      setDevices((previous) =>
        previous.filter((device) => device.id !== selectedDevice.id),
      );

      toast.success(t("Appareil supprimé avec succès."));

      setIsDeleteModalOpen(false);
      setSelectedDevice(null);
    } catch (error) {
      console.error(t("Erreur de suppression de l'appareil :"), error);

      toast.error(
        getErrorMessage(error, t("Impossible de supprimer cet appareil.")),
      );
    } finally {
      setDeleting(false);
    }
  };

  // Recherche par code appareil, agent ou statut
  const normalizedQuery = searchQuery.trim().toLowerCase();

  const filteredAgents = agents.filter((agent) => {
    const query = formData.nameAgent.trim().toLowerCase();

    return (
      String(agent.name ?? "")
        .toLowerCase()
        .includes(query) ||
      String(agent.agentCode ?? "")
        .toLowerCase()
        .includes(query)
    );
  });

  const filteredDevices = devices.filter((device) =>
    [
      device.deviceCode,
      device.agentCode,
      device.isActive ? "actif" : "inactif",
    ].some((field) =>
      String(field ?? "")
        .toLowerCase()
        .includes(normalizedQuery),
    ),
  );

  const formattedCount = String(filteredDevices.length).padStart(2, "0");

  const isFormValid =
    formData.deviceCode.trim() !== "" && formData.nameAgent.trim() !== "";

  // Colonnes du tableau
  const deviceColumns = [
    {
      header: t("Code appareil"),
      accessor: "deviceCode",
    },
    {
      header: t("Matricule agent"),
      accessor: "agentCode",
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
            aria-label={`${t("Modifier")} ${row.deviceCode}`}
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
            aria-label={`${t("Supprimer")} ${row.deviceCode}`}
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
          {t("Gestion des appareils")}
        </h2>

        <Button
          type="button"
          variant="primary"
          onClick={handleOpenCreate}
          className="gap-2 text-sm"
        >
          <span className="text-lg leading-none">+</span>
          {t("Nouvel appareil")}
        </Button>
      </div>

      {/* Recherche */}
      <div className="w-full sm:w-[320px]">
        <Input
          variant="search"
          placeholder={t("Rechercher un appareil...")}
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

      {/* Compteur */}
      <div className="text-sm font-semibold text-gray-800">
        {t("Résultats")} ({formattedCount})
      </div>

      {/* Tableau */}
      {loading ? (
        <p className="py-6 text-sm text-text-secondary">
          {t("Chargement des appareils...")}
        </p>
      ) : filteredDevices.length > 0 ? (
        <DataTable columns={deviceColumns} data={filteredDevices} />
      ) : (
        <p className="py-8 text-sm text-center text-text-secondary">
          {searchQuery.trim()
            ? t("Aucun appareil ne correspond à votre recherche.")
            : t("Aucun appareil enregistré.")}
        </p>
      )}

      {/* Modal de création et de modification */}
      <Modal
        isOpen={isModalOpen}
        onClose={handleCloseModal}
        title={
          editingDevice ? t("Modifier un appareil") : t("Ajout d'un appareil")
        }
        IconTitle={DeviceIcon}
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
                : editingDevice
                  ? t("Enregistrer")
                  : t("Ajouter")}
            </Button>
          </>
        }
      >
        <div className="flex flex-col gap-3">
          <p className="font-sans text-sm text-text-secondary">
            {t(
              "Enregistrez un appareil et associez-le à un agent responsable des rondes de sécurité.",
            )}
          </p>

          <Input
            variant="default"
            label={t("Code de l'appareil")}
            name="deviceCode"
            placeholder={t("Ex. : PN-001")}
            value={formData.deviceCode}
            onChange={handleChangeFor("deviceCode")}
            required
          />

          <div className="relative">
            <Input
              variant="default"
              label={t("Nom de l'agent")}
              name="nameAgent"
              placeholder={t("Rechercher par nom ou matricule...")}
              value={formData.nameAgent}
              onChange={(valueOrEvent) => {
                handleChangeFor("nameAgent")(valueOrEvent);
                setAgentSuggestionsOpen(true);
              }}
              onFocus={() => setAgentSuggestionsOpen(true)}
              required
            />

            {agentSuggestionsOpen && (
              <div className="absolute z-50 w-full mt-1 overflow-y-auto bg-white border border-gray-200 rounded-lg shadow-lg max-h-56">
                {filteredAgents.length > 0 ? (
                  filteredAgents.map((agent) => (
                    <button
                      key={agent.id}
                      type="button"
                      onClick={() => {
                        setFormData((previous) => ({
                          ...previous,
                          nameAgent: agent.name ?? "",
                        }));

                        setAgentSuggestionsOpen(false);
                        setFormError("");
                      }}
                      className="flex items-center justify-between w-full gap-3 px-4 py-3 text-left border-b border-gray-100 last:border-b-0 hover:bg-gray-50"
                    >
                      <span className="text-sm font-medium text-gray-800">
                        {agent.name}
                      </span>

                      <span className="px-2 py-1 text-xs font-semibold text-gray-600 bg-gray-100 rounded-md shrink-0">
                        {agent.agentCode}
                      </span>
                    </button>
                  ))
                ) : (
                  <p className="px-4 py-3 text-sm text-gray-500">
                    {t("Aucun agent trouvé.")}
                  </p>
                )}
              </div>
            )}
          </div>

          {editingDevice && (
            <div className="flex items-center justify-between gap-4 py-2">
              <div>
                <p className="text-sm font-medium text-text-primary">
                  {t("Statut de l’appareil")}
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
                    ? t("Désactiver l’appareil")
                    : t("Activer l’appareil")
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
        IconTitle={DeviceIcon}
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
          {t("Êtes-vous sûr de vouloir supprimer l'appareil")}{" "}
          <strong>{selectedDevice?.deviceCode ?? ""}</strong>?{" "}
          {t("Cette action est irréversible.")}
        </p>
      </Modal>
    </div>
  );
}
