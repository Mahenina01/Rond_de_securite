import React from "react";
import AlertCard, { AlertData } from "./AlertCard";
import frTranslations from "../../constants/alert.json";

const useTranslation = () => {
    return {
        t: (key: string) => {
            const keys = key.split(".");
            let current: any = frTranslations;
            for (const k of keys) {
                current = current?.[k];
            }
            return current || key;
        },
    };
};
export default function AlertList() {
    const { t } = useTranslation();
    const alertsList: AlertData[] = [
        {
            id: 1,
            message: "Ronde incomplète",
            timestamp: "A l'instant",
            location: { name: "Salle 1" },
            agent: {
                agent_code: "AG-024",
                name: "Lova",
                phone: "0341637255",
            },
        },
        {
            id: 2,
            message: "Ronde incomplète",
            timestamp: "hier 13:30",
            location: { name: "Salle 1" },
            agent: {
                agent_code: "AG-012",
                name: "Andry",
                phone: "0324567890",
            },
        },
    ];
    return (
        <div className="min-h-screen bg-gray-100 p-8 flex justify-center items-start">
            <div className="w-[472px] bg-white rounded-[16px] p-6 shadow-sm border border-gray-200 space-y-4">

                {/* En-tête de section avec l'icône Warning et les titres localisés */}
                <div className="flex items-start gap-3">
                    <div className="p-2 rounded-lg bg-amber-50 text-amber-600 flex items-center justify-center">
                        {/* Icône SVG d'avertissement */}
                        <svg
                            xmlns="http://www.w3.org/2000/svg"
                            className="h-6 w-6"
                            fill="none"
                            viewBox="0 0 24 24"
                            stroke="currentColor"
                            strokeWidth={2}
                        >
                            <path
                                strokeLinecap="round"
                                strokeLinejoin="round"
                                d="M12 9v2m0 4h.01m-6.938 4h13.856c1.54 0 2.502-1.667 1.732-3L13.732 4c-.77-1.333-2.694-1.333-3.464 0L3.34 16c-.77 1.333.192 3 1.732 3z"
                            />
                        </svg>
                    </div>
                    <div>
                        <h2 className="text-xl font-bold text-gray-900">{t("alerts.title")}</h2>
                        <p className="text-sm text-gray-500">{t("alerts.subtitle")}</p>
                    </div>
                </div>

                {/* Boucle d'affichage vertical de la liste des cartes d'alertes */}
                <div className="flex flex-col gap-3">
                    {alertsList.map((alert, index) => (
                        <AlertCard
                            key={alert.id}
                            alertData={alert}
                            isDefaultExpanded={index === 0} // Ouvre la première carte par défaut pour correspondre à la maquette
                            t={t}
                        />
                    ))}
                </div>
            </div>
        </div>
    );
}
