import React from "react";

/**
 * Composant réutilisable DataTable
 * Il permet d'afficher n'importe quel tableau de données de manière dynamique 
 * en lui passant les colonnes et les données sous forme de propriétés (props).
 */
export default function DataTable({ columns, data, onEdit, onDelete, onCopy }) {
    return (
        // Conteneur principal avec défilement horizontal si l'écran est trop petit
        <div className="overflow-x-auto w-full">
            <table className="w-full text-left border-collapse">
                
                <thead>
                    <tr className="border-b border-gray-200 text-xs font-semibold text-gray-500 uppercase tracking-wider">
                        {columns.map((col, index) => (
                            <th 
                                key={index} 
                                className={`py-3 px-4 ${col.align === 'right' ? 'text-right' : ''}`}
                            >
                                {col.header}
                            </th>
                        ))}
                    </tr>
                </thead>

                <tbody className="divide-y divide-gray-100 text-sm text-gray-800">
                    {/* Vérification conditionnelle : Est-ce qu'il y a des données à afficher ? */}
                    {data.length > 0 ? (
                        // Si OUI : on parcourt chaque ligne (row) de données
                        data.map((row, rowIndex) => (
                            <tr key={row.id || rowIndex} className="hover:bg-gray-50/50 transition-colors">
                                
                                {/* Pour chaque ligne, on parcourt les colonnes pour afficher les cellules correspondantes */}
                                {columns.map((col, colIndex) => (
                                    <td 
                                        key={colIndex} 
                                        className={`py-4 px-4 ${col.align === 'right' ? 'text-right' : ''}`}
                                    >
                                        {/* 
                                          LOGIQUE CLÉ : 
                                          - Si la colonne possède une fonction 'render' personnalisée (ex: boutons d'action ou icône de copie), 
                                            on l'exécute en lui passant la ligne entière et les fonctions de callback.
                                          - Sinon, on affiche simplement la valeur brute de la propriété (ex: row.name).
                                        */}
                                        {col.render 
                                            ? col.render(row, { onEdit, onDelete, onCopy }) 
                                            : row[col.accessor]
                                        }
                                    </td>
                                ))}
                            </tr>
                        ))
                    ) : (
                        // Si NON : on affiche une ligne unique indiquant qu'aucun résultat n'a été trouvé
                        <tr>
                            <td 
                                colSpan={columns.length} 
                                className="py-6 text-center text-gray-500 text-sm"
                            >
                                Aucun résultat trouvé.
                            </td>
                        </tr>
                    )}
                </tbody>
            </table>
        </div>
    );
}