import { useState } from "react";
import Input from "../../components/ui/Input";
import Button from "../../components/ui/Button";
import AddAgentIcon from "../../assets/icons/user-plus.svg?react"; 

export default function AddAgentModal({ isOpen, onClose, onAddAgent }) {
    const [agentCode, setAgentCode] = useState("");
    const [lastName, setLastName] = useState("");
    const [firstName, setFirstName] = useState("");
    const [phone, setPhone] = useState("");
    const [isSubmitting, setIsSubmitting] = useState(false);

    if (!isOpen) return null;

    const handleSubmit = (e) => {
        e.preventDefault();

        if (!agentCode || !lastName) {
            alert("Veuillez remplir les champs obligatoires (*).");
            return;
        }

        setIsSubmitting(true);

        const newAgent = {
            id: Date.now(),
            agent_code: agentCode,
            name: lastName,
            firstname: firstName,
            phone: phone,
        };

        setTimeout(() => {
            setIsSubmitting(false);
            if (onAddAgent) {
                onAddAgent(newAgent);
            }
            onClose(); 
        }, 1000);
    };

    const isFormValid = agentCode.trim() !== "" && lastName.trim() !== "";

    return (
        <div className="fixed inset-0 z-50 flex items-center justify-center bg-black/40 backdrop-blur-sm p-4">
            
            <div className="w-[522px] max-h-[90vh] overflow-y-auto bg-surface rounded-[32px] px-[30px] py-[35px] shadow-[0_4px_200px_-3px_rgba(0,0,0,0.25)] flex flex-col gap-[20px] animate-in fade-in zoom-in-95 duration-200">
                
                <div className="flex items-center gap-3">
                    <div className="text-primary">
                        <AddAgentIcon className="w-8 h-8" />
                    </div>
                    <h2 className="text-xl font-black text-text-primary tracking-tight">
                        Ajout de nouvel agent
                    </h2>
                </div>

                <div className="flex flex-col gap-1">
                    <p className="text-xs text-text-secondary leading-relaxed">
                        Renseignez les informations nécessaires pour créer la fiche de l'agent et faciliter son identification dans le système de gestion des rondes de sécurité.
                    </p>
                    <span className="text-xs text-red-500 font-medium pt-1">
                         Champs obligatoires
                    </span>
                </div>

                <form onSubmit={handleSubmit} className="flex flex-col gap-[16px]">
                    
                    <Input
                        variant="default"
                        label="Matricule de l'agent "
                        name="agentCode"
                        placeholder="Ex. : AG-00125"
                        value={agentCode}
                        onChange={(e) => setAgentCode(e.target.value)}
                        required={true}
                    />

                    <Input
                        variant="default"
                        label="Nom "
                        name="lastName"
                        placeholder="Saisissez le nom de famille"
                        value={lastName}
                        onChange={(e) => setLastName(e.target.value)}
                        required={true}
                    />

                    <Input
                        variant="default"
                        label="Prénoms (facultatif)"
                        name="firstName"
                        placeholder="Saisissez le ou les prénoms"
                        value={firstName}
                        onChange={(e) => setFirstName(e.target.value)}
                    />

                    <Input
                        variant="phone"
                        label="Téléphone (facultatif)"
                        name="phone"
                        placeholder="Ex. : +261 34 00 000 00"
                        value={phone}
                        onChange={(e) => setPhone(e.target.value)}
                    />

                    <div className="flex items-center justify-between gap-4 pt-4">
                        <Button
                            type="button"
                            variant="outline"
                            className="w-1/2 justify-center"
                            onClick={onClose}
                        >
                            Annuler
                        </Button>
                        <Button
                            type="submit"
                            variant="primary"
                            state={isSubmitting ? "waiting" : "default"}
                            disabled={!isFormValid || isSubmitting}
                            className="w-1/2 justify-center text-xs"
                        >
                            {isSubmitting ? "Création..." : "Créer l'agent"}
                        </Button>
                    </div>

                </form>
            </div>
        </div>
    );
}