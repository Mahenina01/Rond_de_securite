import cardTextVariant from "../../constants/cardText";

const Card = ({
  variant = "ronde_confirm",
  valueCount = "-----",
  evolution,
}) => {
  const config = cardTextVariant[variant];

  if (!config) return null;

  const Icon = config.icon;

  return (
    <div className="flex h-[175px] w-[300px] flex-col rounded-[20px] bg-surface p-5">
      <div className="flex flex-row items-center justify-between gap-2">
        <h2 className="font-sans font-semibold text-text-primary">
          {config.text}
        </h2>

        <Icon className={`h-10 w-10 shrink-0 ${config.color}`} />
      </div>

      <h1 className="flex items-center flex-1 text-4xl font-black">
        {valueCount}
      </h1>

      <p className="text-sm text-text-secondary">
        <span className="text-success">+{evolution}%</span> vs hier
      </p>
    </div>
  );
};

export default Card;
