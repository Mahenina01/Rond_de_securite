import React, { useState } from "react";
import Input from "../../components/ui/Input"
import Button from "../../components/ui/button"
import ForgotPassword from "../../assets/icons/forgot-password.svg?react"


export default function ForgotPasswordPage() {
    const [email, setEmail] = useState("");
    const [isSubmitting, setIsSubmitting] = useState(false);


    const handleSubmit = (e) => {
        e.preventDefault();
        setIsSubmitting(true);

        console.log("Envoi du code de vérification à l'adresse :", email);

        setTimeout(() => {
            setIsSubmitting(false);
            alert(`Un code de vérification a été envoyé à ${email}`);
        }, 1500)
    };
    return (
        <main className="min-h-screen bg-background flex item-center justify-center p-4">
            <div className="w-[522px] bg-surface rounded-[32px] px-[30px] py-[40px] shadow-[0_4px_200px_-3px_rgba(0,0,0,0.25)] flex flex-col gap-[24px]">
                <div className="flex justify-center py-2">
                    <ForgotPassword className="w-[212px] h-[171px] object-contain text-primary" />
                </div>
                <div className="flex flex-col text-center gap-2">
                    <h1 className="text-xl font-black text-text-primary tracking-tight">
                        MOT DE PASSE OUBLIÉ ?
                    </h1>
                    <p className="text-sm text-text-secondary leading-relaxed px-4 text-justify">
                        Pas d'inquiétude. Saisissez votre adresse e-mail et nous vous enverrons un code de vérification pour réinitialiser votre mot de passe.
                    </p>
                </div>
                <form onSubmit={handleSubmit} className="flex flex-col gap-[24px">
                    <Input
                        variant="email"
                        label="Adresse E-mail"
                        name="email"
                        placeholder="example0gmail.com"
                        value={email}
                        onChange={(e) => setEmail(e.target.value)}
                        required={true}
                    />
                    <div className="flex items-center justify-between gap-4 pt-2">
                        <Button
                            type="button"
                            variant="outline"
                            className="w-1/2 justify-center"
                        >Retour</Button>
                        <Button
                            type="submit"
                            variant="primary"
                            state={isSubmitting ? "waiting" : "default"}
                            disabled={!email || isSubmitting}
                            className="w-1/2 justify-center"
                        >
                            {isSubmitting ? "Envoi en cours..." : "Envoyer"}

                        </Button>
                    </div>
                </form>
            </div>
        </main>
    )
}