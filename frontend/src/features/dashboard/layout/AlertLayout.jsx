import AlertIcon from "../../../assets/icons/alert 1.svg?react";
import AlertCard from "../../../components/ui/AlertCard";

const AlertLayout = () => {
  const alertsData = [
    {
      message: "Passage non détecté",
      timestamp: "2026-10-14T14:30:00",
      location: { name: "Point de contrôle A" },
      agent: {
        agent_code: "AG-001",
        name: "Jean",
        phone: "0341234567",
      },
    },
    {
      message: "Retard de passage",
      timestamp: "2026-10-14T15:45:00",
      location: { name: "Entrée principale" },
      agent: {
        agent_code: "AG-002",
        name: "Marie",
        phone: "0321234567",
      },
    },
    {
      message: "Anomalie détectée pendant la ronde",
      timestamp: "2026-10-14T16:20:00",
      location: { name: "Point de contrôle B" },
      agent: {
        agent_code: "AG-003",
        name: "Andry",
        phone: "0331234567",
      },
    },
  ];

  return (
    <div className="flex h-[380px] min-h-0 flex-col gap-3 overflow-hidden rounded-lg bg-surface p-4">
      {/* Titre fixe */}
      <div className="flex flex-row items-center gap-2 shrink-0">
        <AlertIcon className="w-8 h-8 text-warning" />
        <h1 className="font-sans text-2xl text-text-primary">Alertes</h1>
      </div>

      {/* Description fixe */}
      <p className="font-sans text-sm shrink-0 text-text-secondary">
        Derniers événements détectés sur le terrain.
      </p>

      {/* Seule cette zone défile */}
      <div className="flex flex-col flex-1 min-h-0 gap-3 overflow-y-auto">
        {alertsData.map((alert, index) => (
          <AlertCard
            key={`${alert.agent.agent_code}-${index}`}
            alertData={alert}
          />
        ))}
      </div>
    </div>
  );
};

export default AlertLayout;
