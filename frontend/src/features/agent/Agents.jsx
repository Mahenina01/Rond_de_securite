import { useState } from "react";
import Input from "../../components/ui/Input";
import IconButton from "../../components/ui/IconButton";
import AddIcon from "../../assets/icons/add.svg?react";
import Delete from "../../assets/icons/delete.svg?react";
import DataTable from "../../components/ui/DataTable";
import Modal from "../../components/ui/Modal";
import Button from "../../components/ui/Button";
import UserIcon from "../../assets/icons/user-round-plus.svg?react";

const initialAgents = [
  { id: 1, idagent: "AG-024", name: "Luca RJ", phone: "0347722626" },
  { id: 2, idagent: "AG-043", name: "Jhon Doe", phone: "0335363666" },
  { id: 3, idagent: "AG-054", name: "Henri Rakoto", phone: "0317736256" },
  { id: 4, idagent: "AG-055", name: "Henri Razafy", phone: "0342416424" },
  { id: 5, idagent: "AG-0541", name: "Feno Hery", phone: "0342254253" },
  { id: 6, idagent: "AG-056", name: "Mialy Soa", phone: "0321145678" },
  { id: 7, idagent: "AG-057", name: "Tiana Rabe", phone: "0339876543" },
  { id: 8, idagent: "AG-058", name: "Andry Lova", phone: "0341234567" },
  { id: 9, idagent: "AG-059", name: "Nirina Kanto", phone: "0328901234" },
  { id: 10, idagent: "AG-060", name: "Hery Zo", phone: "0334567890" },
];

const emptyForm = {
  idagent: "",
  nom: "",
  prenoms: "",
  phone: "",
};

const Agents = () => {
  const [isModalOpen, setIsModalOpen] = useState(false);
  const [searchQuery, setSearchQuery] = useState("");
  const [agents, setAgents] = useState(initialAgents);
  const [formData, setFormData] = useState(emptyForm);
  const [formError, setFormError] = useState("");

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

  const handleConfirm = () => {
    const idagent = formData.idagent.trim().toUpperCase();
    const nom = formData.nom.trim();
    const prenoms = formData.prenoms.trim();
    const phone = formData.phone.trim();

    if (!idagent || !nom) {
      setFormError("Le matricule et le nom sont obligatoires.");
      return;
    }

    const alreadyExists = agents.some(
      (agent) => agent.idagent.toLowerCase() === idagent.toLowerCase(),
    );

    if (alreadyExists) {
      setFormError("Ce matricule existe déjà.");
      return;
    }

    const newAgent = {
      id:
        agents.length > 0
          ? Math.max(...agents.map((agent) => agent.id)) + 1
          : 1,
      idagent,
      name: [nom, prenoms].filter(Boolean).join(" "),
      phone,
    };

    setAgents((previous) => [...previous, newAgent]);
    setSearchQuery("");
    handleCloseModal();
  };

  const handleDelete = (id) => {
    setAgents((previous) => previous.filter((agent) => agent.id !== id));
  };

  const handleEdit = (agent) => {
    alert(`Modification de l'agent : ${agent.name}`);
  };

  const normalizedQuery = searchQuery.trim().toLowerCase();

  const filteredAgent = agents.filter((agent) =>
    [agent.name, agent.idagent, agent.phone].some((field) =>
      String(field ?? "")
        .toLowerCase()
        .includes(normalizedQuery),
    ),
  );

  const formattedCount = String(filteredAgent.length).padStart(2, "0");

  const isFormValid =
    formData.idagent.trim().length > 0 && formData.nom.trim().length > 0;

  const agentColumns = [
    { header: "ID Agent", accessor: "idagent" },
    { header: "Nom Complet", accessor: "name" },
    { header: "Téléphone", accessor: "phone" },
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
        Gestion des Agents
      </h1>

      <div className="flex flex-row items-center justify-between w-full gap-4">
        <div className="w-full sm:w-[320px]">
          <Input
            variant="search"
            placeholder="Recherche..."
            name="searchQuery"
            value={searchQuery}
            onChange={(value) => {
              setSearchQuery(
                typeof value === "string" ? value : value.target.value,
              );
            }}
          />
        </div>

        <IconButton
          isButton={true}
          iconSVG={AddIcon}
          text="Nouvel agent"
          onClick={() => setIsModalOpen(true)}
        />

        <Modal
          isOpen={isModalOpen}
          onClose={handleCloseModal}
          title="Ajout d'un nouvel agent"
          IconTitle={UserIcon}
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
                Ajouter
              </Button>
            </>
          }
        >
          <div className="flex flex-col gap-3">
            <p className="font-sans text-sm text-text-secondary">
              Renseignez les informations nécessaires pour créer la fiche de
              l'agent et faciliter son identification.
            </p>

            <Input
              variant="default"
              label="Matricule de l'agent"
              placeholder="Ex. : AG-061"
              name="idagent"
              value={formData.idagent}
              onChange={handleChangeFor("idagent")}
              required={true}
            />

            <Input
              variant="default"
              label="Nom"
              placeholder="Saisissez le nom de famille"
              name="nom"
              value={formData.nom}
              onChange={handleChangeFor("nom")}
              required={true}
            />

            <Input
              variant="default"
              label="Prénom(s) (facultatif)"
              placeholder="Saisissez le ou les prénoms"
              name="prenoms"
              value={formData.prenoms}
              onChange={handleChangeFor("prenoms")}
              required={false}
            />

            <Input
              variant="default"
              label="Téléphone (facultatif)"
              placeholder="Ex. : 0341234567"
              name="phone"
              value={formData.phone}
              onChange={handleChangeFor("phone")}
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

      <DataTable columns={agentColumns} data={filteredAgent} />
    </div>
  );
};

export default Agents;
