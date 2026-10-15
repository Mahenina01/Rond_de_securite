import StatCard from "./layout/StatCard";
import WeeklyChart from "../../components/charts/WeeklyChart";
import AlertLayout from "./layout/AlertLayout";
import IconButton from "../../components/ui/IconButton";
import TableLayout from "./layout/TableLayout";
import agentCheckpoints from "./data/agentCheckPoint";

const Dashboard = () => {
  return (
    <div className="flex flex-col gap-6">
      <StatCard />

      <div className="flex flex-row gap-6">
        <div className="flex flex-col flex-1 gap-4 p-4 rounded-lg bg-surface">
          <h1 className="font-sans text-xl text-text-primary">
            Passages confirmés
          </h1>
          <p className="font-sans text-sm text-text-secondary">
            Évolution sur les 7 derniers jours
          </p>
          <WeeklyChart />
        </div>

        <AlertLayout />
      </div>
      <div className="flex flex-col flex-1 gap-4 p-4 rounded-lg bg-surface">
        <div className="flex flex-row items-center">
          <div className="flex flex-col flex-1 gap-4">
            <h1 className="font-sans text-xl text-text-primary">
              Logs récents
            </h1>
            <p className="font-sans text-sm text-text-secondary">
              Derniers événements enregistrés par les dispositifs
            </p>
          </div>
          <IconButton className="w-auto" isButton={true} text="Exporter logs" />
        </div>
        <TableLayout data={agentCheckpoints} />
      </div>
    </div>
  );
};

export default Dashboard;
