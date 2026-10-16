import { useState } from "react";
import Input from "../../components/ui/Input";
import IconButton from "../../components/ui/IconButton";
import AddIcon from "../../assets/icons/add.svg?react";
import Delete from "../../assets/icons/delete.svg?react";
import DataTable from "../../components/ui/DataTable";
import AddAgentModal from "./AddAgentModal";

const agentsList = [
  { id: 1, idagent: "AG-024", name: "Luca RJ", phone: "0347722626" },
  { id: 2, idagent: "AG-043", name: "Jhon Doe", phone: "0335363666" },
  { id: 3, idagent: "AG-054", name: "Henri Rakoto", phone: "0317736256" },
  { id: 4, idagent: "AG-055", name: "Henri Razafy", phone: "0342416424" },
  { id: 5, idagent: "AG-54", name: "Feno Hery", phone: "0342254253" },
  { id: 6, idagent: "AG-056", name: "Mialy Soa", phone: "0321145678" },
  { id: 7, idagent: "AG-057", name: "Tiana Rabe", phone: "0339876543" },
  { id: 8, idagent: "AG-058", name: "Andry Lova", phone: "0341234567" },
  { id: 9, idagent: "AG-059", name: "Nirina Kanto", phone: "0328901234" },
  { id: 10, idagent: "AG-060", name: "Hery Zo", phone: "0334567890" },
];

const Agents = () => {
  const [searchQuery, setSearchQuery] = useState("");

  const [agent, setAgent] = useState(agentsList);
  const [isModalOpen, setIsModalOpen]= useState(false);
  const handleDelete = (id) => {
    setAgent(agent.filter((device) => device.id !== id));
  };

  const handleEdit = (agent) => {
    alert(`Modification de l'appareil : ${agent.name}`);
  };
  const handleAddAgent =(newAgentData)=>{
    const formattedNewAgent = {
      id: Date.now(),
      idagent: newAgentData.agent_code,
      name: `${newAgentData.name} ${newAgentData.firstname || ""}`.trim(),
      phone: newAgentData.phone || "Non renseigné",
    };
    setAgent([formattedNewAgent, ...agent]);
  };

  // 3. Filtrage dynamique selon la recherche
  const filteredAgent = agent.filter(
    (device) =>
      device.name.toLowerCase().includes(searchQuery.toLowerCase()) ||
      device.agentId.toLowerCase().includes(searchQuery.toLowerCase()),
  );

  // Formatage du compteur avec un zéro devant si < 10 (ex: 05)
  const formattedCount = String(filteredAgent.length).padStart(2, "0");

  // 4. Configuration des colonnes du tableau (avec rendu personnalisé pour les clés et les actions)
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
    <div className="flex flex-col gap-6 p-4 rounded-lg bg-surface">
      <h1 className="font-sans text-xl text-text-primary">
        Gestion des Agents
      </h1>
      <div>
        <div className="w-full sm:w-[320px]">
          <Input
            variant="search"
            placeholder="Recherche..."
            value={searchQuery}
            onChange={(e) => setSearchQuery(e.target.value)}
          />
        </div>
        <IconButton isButton={true} iconSVG={AddIcon} text="Nouveau agents" onClick={()=> setIsModalOpen(true)}/>
      </div>
      <div className="text-sm font-semibold text-gray-800">
        Résultats ({formattedCount})
      </div>
      <DataTable columns={agentColumns} data={filteredAgent} />
      <AddAgentModal
        isOpen={isModalOpen} 
        onClose={()=>setIsModalOpen(false)}
        onAddAgent={handleAddAgent}
      />
    </div>
  );
};

export default Agents;
