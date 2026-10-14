import { useState } from "react";
import { formatPhone, formatTime } from "../../utils/formatters";
import alerts from "../../constants/alert";

const AlertCard = ({ alertData }) => {
  const [isExpanded, setIsExpanded] = useState(false);

  const agent = alertData?.agent;
  const location = alertData?.location;

  const handleCardClick = () => {
    setIsExpanded((previous) => !previous);
  };

  return (
    <article
      onClick={handleCardClick}
      onKeyDown={(event) => {
        if (event.key === "Enter" || event.key === " ") {
          event.preventDefault();
          handleCardClick();
        }
      }}
      role="button"
      tabIndex={0}
      aria-expanded={isExpanded}
      style={{ width: "100%", maxWidth: "400px" }}
      className={`cursor-pointer select-none rounded-[10px] p-[10px] transition-all duration-300 ease-in-out ${
        isExpanded
          ? "border border-[#EDEDED] bg-white shadow-sm"
          : "border border-[#FF8D28] bg-[#D39208]/[0.05]"
      }`}
    >
      <div className="flex items-start justify-between gap-3">
        <span className="text-sm font-semibold text-gray-900">
          {alertData?.message ?? "Aucun message"}
        </span>

        <span className="text-sm font-medium text-blue-600 shrink-0">
          {formatTime(alertData?.timestamp ?? "")}
        </span>
      </div>

      <div className="mt-1 text-sm text-gray-600">
        {location?.name ?? "Emplacement inconnu"}
      </div>

      <div
        className={`grid overflow-hidden transition-all duration-300 ease-in-out ${
          isExpanded
            ? "mt-3 grid-rows-[1fr] opacity-100"
            : "mt-0 grid-rows-[0fr] opacity-0"
        }`}
      >
        <div className="overflow-hidden">
          <div className="space-y-1.5 rounded-[8px] border border-[#C4B5FD] bg-[#F5F3FF]/50 p-3 text-xs text-gray-800">
            <div>
              <span className="font-medium text-gray-600">
                {alerts.agentId}
              </span>{" "}
              <span className="font-semibold text-gray-900">
                {agent?.agent_code ?? "—"}
              </span>
            </div>

            <div>
              <span className="font-medium text-gray-600">
                {alerts.agentName}
              </span>{" "}
              <span className="font-semibold text-gray-900">
                {agent?.name ?? "—"}
              </span>
            </div>

            <div>
              <span className="font-medium text-gray-600">
                {alerts.location}
              </span>{" "}
              <span className="font-semibold text-gray-900">
                {location?.name ?? "—"}
              </span>
            </div>

            <div>
              <span className="font-medium text-gray-600">{alerts.time}</span>{" "}
              <span className="font-semibold text-gray-900">
                {formatTime(alertData?.timestamp ?? "")}
              </span>
            </div>

            <div>
              <span className="font-medium text-gray-600">{alerts.phone}</span>{" "}
              <span className="font-semibold text-gray-900">
                {agent?.phone ? formatPhone(agent.phone) : "—"}
              </span>
            </div>
          </div>
        </div>
      </div>
    </article>
  );
};

export default AlertCard;
