import React, { useState } from "react";
import Input from "../../components/ui/Input";
import Button from "../../components/ui/button";
// Importe l'icône SVG de l'ordinateur (assure-toi d'avoir ton fichier ou adapte le chemin)
import ResetPasswordIcon from "../../assets/icons/secure-password.svg?react";

export default function ResetPasswordPage(){
    const [password, setPassword] = useState("");
    const [confirmPassword, setConfirmPassword] = useState("");
    const [isSubmitting, setIsSubmitting] = useState(false);

    const handleSubmit = (e)=>{
        e.preventDefault();
        if(password !== confirmPassword){
            alert("Les mots de passe ne correspondent pas.");
            return;
        }
        if(password.length<8){
            alert("Le mot de passe doit contenir au moins 8 caractères.");
            return;
        }
        setIsSubmitting(true);
        console.log("Mise à jour du mot de passe en cours...");

        setTimeout(() => {
            setIsSubmitting(false);
            alert("Votre mot de passe a été réinitialisé avec succès !");
        }, 1500);

    };
    const isFormValid = password.length >= 8 && password === confirmPassword;
    return (
        <main className="min-h-screen bg-background flex items-center justify-center p-4">
            <div className="w-[522px] bg-surface rounded-[32px] px-[30px] py-[40px] shadow-[0_4px_200px_-3px_rgba(0,0,0,0.25)] flex flex-col gap-[24px]">
                
                <div className="flex justify-center items-center py-2">
                    <ResetPasswordIcon 
                        className="w-[212px] h-[171px] object-contain text-primary" 
                    />
                </div>

                <div className="flex flex-col text-center gap-2">
                    <h1 className="text-xl font-black text-text-primary tracking-tight">
                        CRÉEZ UN NOUVEAU MOT DE PASSE
                    </h1>
                    <p className="text-sm text-text-secondary leading-relaxed px-2">
                        Votre adresse e-mail a été vérifiée. Choisissez un nouveau mot de passe sécurisé pour retrouver l'accès à votre compte.
                    </p>
                </div>

                <form onSubmit={handleSubmit} className="flex flex-col gap-[20px]">
                    
                    <Input
                        variant="password"
                        label="Nouveau mot de passe"
                        name="password"
                        placeholder="••••••••"
                        value={password}
                        onChange={(e) => setPassword(e.target.value)}
                        required={true}
                    />

                    <Input
                        variant="password"
                        label="Confirmer mot de passe"
                        name="confirmPassword"
                        placeholder="••••••••"
                        value={confirmPassword}
                        onChange={(e) => setConfirmPassword(e.target.value)}
                        required={true}
                    />

                    <p className="text-xs text-text-secondary text-justify px-4 leading-normal">
                        Votre mot de passe doit contenir au moins 8 caractères. Utilisez une combinaison de lettres, de chiffres et de caractères spéciaux pour renforcer sa sécurité.
                    </p>

                    <div className="flex items-center justify-between gap-4 pt-2">
                        <Button
                            type="button"
                            variant="outline"
                            className="w-1/2 justify-center"
                            onClick={() => console.log("Retour en arrière")}
                        >
                            Retour
                        </Button>
                        <Button
                            type="submit"
                            variant="primary"
                            state={isSubmitting ? "waiting" : "default"}
                            disabled={!isFormValid || isSubmitting}
                            className="w-1/2 justify-center text-xs"
                        >
                            {isSubmitting ? "Mise à jour..." : "Réinitialiser le mot de passe"}
                        </Button>
                    </div>

                </form>
            </div>
        </main>
    );
}
