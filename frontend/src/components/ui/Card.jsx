import cardTextVariant from "../../constants/cardText";
import useLanguage from "../../context/useLanguage";

const Card = ({
  variant = "ronde_confirm",
  valueCount = "-----",
  evolution = null,
}) => {
  const config = cardTextVariant[variant];
  const { t } = useLanguage();

  if (!config) return null;

  const Icon = config.icon;
  const hasEvolution = typeof evolution === "number" && Number.isFinite(evolution);
  const formattedEvolution = hasEvolution
    ? `${evolution > 0 ? "+" : ""}${evolution.toFixed(1)}%`
    : "—";

  return (
    <div className="flex gap-4 flex-col rounded-[20px] bg-surface p-5">
      <div className="flex flex-row items-center justify-between gap-2">
        <h2 className="font-sans font-semibold text-text-primary">
          {t(config.text)}
        </h2>

        <Icon className={`h-10 w-10 shrink-0 ${config.color}`} />
      </div>

      <h1 className="flex items-center flex-1 text-4xl font-black">
        {valueCount}
      </h1>

      <p className="text-sm text-text-secondary">
        <span className={hasEvolution && evolution < 0 ? "text-red-600" : "text-success"}>
          {formattedEvolution}
        </span>{" "}
        {t("vs hier")}
      </p>
    </div>
  );
};

export default Card;
