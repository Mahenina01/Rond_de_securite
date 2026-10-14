import Card from "../../../components/ui/Card";
const StatCard = () => {
  return (
    <div className="flex flex-row items-center justify-between">
      <Card variant="ronde_confirm" />
      <Card variant="device_actif_count" />
      <Card variant="succes_percent" />
    </div>
  );
};

export default StatCard;
