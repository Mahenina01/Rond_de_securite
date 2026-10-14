import Card from "../../../components/ui/Card";

const StatCard = () => {
  return (
    <div className="grid grid-cols-1 gap-6 sm:grid-cols-2 lg:grid-cols-3">
      <Card variant="ronde_confirm" />
      <Card variant="device_actif_count" />
      <Card variant="succes_percent" />
    </div>
  );
};

export default StatCard;
