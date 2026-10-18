import { useCallback, useEffect, useState } from "react";
import Button from "../../components/ui/Button";
import Input from "../../components/ui/Input";
import DataTable from "../../components/ui/DataTable";
import Delete from "../../assets/icons/delete.svg?react";
import Modal from "../../components/ui/Modal";
import DeviceIcon from "../../assets/icons/device-out.svg?react";
import devicesService from "../../service/devciesService";
import toast from "react-hot-toast";

const emptyForm = {
  deviceCode: "",
  nameAgent: "",
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

  // Charger les appareils depuis le backend
  const loadDevices = useCallback(async () => {
    setLoading(true);

    try {
      const response = await devicesService.getAll();
      const list = getResponseList(response);

      if (!list) {
        throw new Error(
          "Le serveur n'a pas renvoyé une liste valide d'appareils.",
        );
      }

      setDevices(list);
    } catch (error) {
      console.error("Erreur de chargement des appareils :", error);

      toast.error(
        getErrorMessage(error, "Impossible de charger les appareils."),
      );
    } finally {
      setLoading(false);
    }
  }, []);

  useEffect(() => {
    loadDevices();
  }, [loadDevices]);

  // Réinitialiser le formulaire
  const resetForm = () => {
    setFormData(emptyForm);
    setFormError("");
    setEditingDevice(null);
  };

  // Fermer le formulaire
  const handleCloseModal = () => {
    if (submitting) return;

    setIsModalOpen(false);
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
        "Le code de l'appareil et le nom de l'agent sont obligatoires.",
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
      setFormError("Ce code appareil existe déjà.");
      return;
    }

    setSubmitting(true);
    setFormError("");

    // Conserver le statut existant en modification
    /*const devicePayload = {
      ...payload,
      isActive: editingDevice ? editingDevice.isActive : true,
    };*/

    try {
      if (editingDevice) {
        await devicesService.update(editingDevice.id, payload);
        toast.success("Appareil modifié avec succès.");
      } else {
        await devicesService.create(payload);
        toast.success("Appareil créé avec succès.");
      }

      setIsModalOpen(false);
      resetForm();

      await loadDevices();
    } catch (error) {
      console.error("Erreur d'enregistrement de l'appareil :", error);

      const message = getErrorMessage(
        error,
        editingDevice
          ? "Impossible de modifier cet appareil."
          : "Impossible de créer cet appareil.",
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

      toast.success("Appareil supprimé avec succès.");

      setIsDeleteModalOpen(false);
      setSelectedDevice(null);
    } catch (error) {
      console.error("Erreur de suppression de l'appareil :", error);

      toast.error(
        getErrorMessage(error, "Impossible de supprimer cet appareil."),
      );
    } finally {
      setDeleting(false);
    }
  };

  // Recherche par code appareil, agent ou statut
  const normalizedQuery = searchQuery.trim().toLowerCase();

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
      header: "Code appareil",
      accessor: "deviceCode",
    },
    {
      header: "Matricule agent",
      accessor: "agentCode",
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
            aria-label={`Modifier ${row.deviceCode}`}
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
            aria-label={`Supprimer ${row.deviceCode}`}
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
          type="button"
          variant="primary"
          onClick={handleOpenCreate}
          className="gap-2 text-sm"
        >
          <span className="text-lg leading-none">+</span>
          Nouvel appareil
        </Button>
      </div>

      {/* Recherche */}
      <div className="w-full sm:w-[320px]">
        <Input
          variant="search"
          placeholder="Rechercher un appareil..."
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
        Résultats ({formattedCount})
      </div>

      {/* Tableau */}
      {loading ? (
        <p className="py-6 text-sm text-text-secondary">
          Chargement des appareils...
        </p>
      ) : filteredDevices.length > 0 ? (
        <DataTable columns={deviceColumns} data={filteredDevices} />
      ) : (
        <p className="py-8 text-sm text-center text-text-secondary">
          {searchQuery.trim()
            ? "Aucun appareil ne correspond à votre recherche."
            : "Aucun appareil enregistré."}
        </p>
      )}

      {/* Modal de création et de modification */}
      <Modal
        isOpen={isModalOpen}
        onClose={handleCloseModal}
        title={editingDevice ? "Modifier un appareil" : "Ajout d'un appareil"}
        IconTitle={DeviceIcon}
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
                : editingDevice
                  ? "Enregistrer"
                  : "Ajouter"}
            </Button>
          </>
        }
      >
        <div className="flex flex-col gap-3">
          <p className="font-sans text-sm text-text-secondary">
            Enregistrez un appareil et associez-le à un agent responsable des
            rondes de sécurité.
          </p>

          <Input
            variant="default"
            label="Code de l'appareil"
            name="deviceCode"
            placeholder="Ex. : PN-001"
            value={formData.deviceCode}
            onChange={handleChangeFor("deviceCode")}
            required
          />

          <Input
            variant="default"
            label="Nom de l'agent"
            name="nameAgent"
            placeholder="Ex. : Jean Doe"
            value={formData.nameAgent}
            onChange={handleChangeFor("nameAgent")}
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
        IconTitle={DeviceIcon}
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
          Êtes-vous sûr de vouloir supprimer l'appareil{" "}
          <strong>{selectedDevice?.deviceCode ?? ""}</strong>? Cette action est
          irréversible.
        </p>
      </Modal>
    </div>
  );
}
