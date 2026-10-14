
import React, { useState } from "react";
import { formatPhone } from "../../utils/formatters"

export interface AlertData {
    id: number | string;
    message: string;
    timestamp: string;
    location: {
        name: string;
    };
    agent: {
        agent_code: string;
        name: string;
        phone: string;
    };
}

    interface AlertCardProps {
    alertData: AlertData;
    isDefaultExpanded?: boolean;
    onToggle?: (expanded: boolean) => void;
    t: (key: string) => string;
}

export default function AlertCard({
    alertData,
    isDefaultExpanded = false,
    onToggle,
    t,
}: AlertCardProps) {
    const [isExpanded, setIsExpanded] = useState<boolean>(isDefaultExpanded);

    const handleCardClick = () => {
        const nextState = !isExpanded;
        setIsExpanded(nextState);
        if (onToggle) {
            onToggle(nextState)
        }
    };

    return (
        <div
            onClick={handleCardClick}
            style={{ width: "424px" }}
            className={`cursor-pointer rounded-[10px] p-[10px] gap-[10px] transition-all duration-300 ease-in-out select-none ${isExpanded
                ? "bg-white border border-[#EDEDED] shadow-sm" // Style État étendu (alert2.png)
                : "bg-[#D39208]/[0.05] border border-[#FF8D28]" // Style État fermé (alert1.png)
                }`}
        >
            <div className="flex justify-between items-start">
                <span className="font-semibold text-gray-900 text-sm">
                    {alertData.message}
                </span>
                <span className="text-blue-600 text-sm font-medium">
                    {alertData.timestamp}
                </span>
            </div>
            <div className="text-gray-600 text-sm mt-1">
                {alertData.location.name}
            </div>
            <div
                className={`grid transition-all duration-300 ease-in-out overflow-hidden ${isExpanded
                    ? "grid-rows-[1fr] opacity-100 mt-3"
                    : "grid-rows-[0fr] opacity-0 mt-0"
                    }`}
            >

                <div className="overflow-hidden">
                    {/* Conteneur secondaire avec un encadré violet clair */}
                    <div className="rounded-[8px] border border-[#C4B5FD] bg-[#F5F3FF]/50 p-3 text-xs text-gray-800 space-y-1.5">
                        <div>
                            <span className="font-medium text-gray-600">{t("alerts.agentId")}</span>{" "}
                            <span className="font-semibold text-gray-900">{alertData.agent.agent_code}</span>
                        </div>
                        <div>
                            <span className="font-medium text-gray-600">{t("alerts.agentName")}</span>{" "}
                            <span className="font-semibold text-gray-900">{alertData.agent.name}</span>
                        </div>
                        <div>
                            <span className="font-medium text-gray-600">{t("alerts.location")}</span>{" "}
                            <span className="font-semibold text-gray-900">{alertData.location.name}</span>
                        </div>
                        <div>
                            <span className="font-medium text-gray-600">{t("alerts.time")}</span>{" "}
                            <span className="font-semibold text-gray-900">{alertData.timestamp}</span>
                        </div>
                        <div>
                            <span className="font-medium text-gray-600">{t("alerts.phone")}</span>{" "}
                            <span className="font-semibold text-gray-900">{formatPhone(alertData.agent.phone)}</span>
                        </div>
                    </div>
                </div>
            </div>


        </div>

    )
}

