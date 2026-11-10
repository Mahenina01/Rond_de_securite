import StatCard from "./layout/StatCard";
import WeeklyChart from "../../components/charts/WeeklyChart";
import AlertLayout from "./layout/AlertLayout";
import IconButton from "../../components/ui/IconButton";
import TableLayout from "./layout/TableLayout";
import agentCheckpoints from "./data/agentCheckPoint";
import useLanguage from "../../context/useLanguage";

const Dashboard = () => {
  const { t } = useLanguage();

  return (
    <div className="flex flex-col gap-6">
      <StatCard />

      <div className="flex flex-col gap-6 xl:flex-row">
        <div className="flex w-full min-w-0 flex-col gap-4 rounded-lg bg-surface p-4 xl:flex-1">
          <h1 className="font-sans text-xl text-text-primary">
            {t("Passages confirmés")}
          </h1>
          <p className="font-sans text-sm text-text-secondary">
            {t("Évolution sur les 7 derniers jours")}
          </p>
          <WeeklyChart />
        </div>

        <div className="w-full xl:w-[360px] xl:shrink-0">
          <AlertLayout />
        </div>
      </div>
      <div className="flex flex-col flex-1 gap-4 p-4 rounded-lg bg-surface">
        <div className="flex flex-row items-center">
          <div className="flex flex-col flex-1 gap-4">
            <h1 className="font-sans text-xl text-text-primary">
              {t("Logs récents")}
            </h1>
            <p className="font-sans text-sm text-text-secondary">
              {t("Derniers événements enregistrés par les dispositifs")}
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
